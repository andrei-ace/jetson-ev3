#include "BattleTank.hpp"

#include "engine/core/logger.hpp"
#include <math.h>

namespace isaac
{
namespace ev3
{

int writePPM(uint16_t width, uint16_t height, uint32_t *image, const char *filename, uint32_t index);
void demosaic(uint16_t width, uint16_t height, const uint8_t *bayerImage, uint32_t *image);

// Name of states
constexpr char kStateInit[] = "kInit";
constexpr char kStateExit[] = "kExit";
constexpr char kStateNavigation[] = "kStateNavigation";
constexpr char kStateDetected[] = "kStateDetected";
constexpr char kStateCloseToShoot[] = "kStateCloseToShoot";
constexpr char kStateShoot[] = "kStateShoot";

void BattleTank::start()
{
    navigation_mode_ = node()->app()->findComponentByName<navigation::GroupSelectorBehavior>(
        get_navigation_mode());
    ASSERT(navigation_mode_, "Could not find navigation mode");
    createStateMachine();

    // Start in desired state
    machine_.start(kStateInit);

    tickPeriodically();

    // we need to initalise it after call to tickPeriodically() to work
    int result = pixy.init();
    ASSERT(result >= 0, "Could not initialise Pixy2 Camera");
}

void BattleTank::tick()
{
    pixy.ccc.getBlocks(true, CCC_SIG1, 1);
    machine_.tick();
}

void BattleTank::stop()
{
    machine_.stop();
}

void BattleTank::createStateMachine()
{
    const std::vector<State> all_states = {
        kStateNavigation,
        kStateDetected,
        kStateShoot};

    machine_.setToString([this](const State &state) { return state; });

    machine_.addState(kStateInit, [] {}, [] {}, [] {});

    machine_.addTransition(kStateInit, kStateNavigation,
                           [this] {
                               bool ok;
                               Pose2d pose = get_world_T_robot(getTickTime(), ok);
                               if (ok)
                               {
                                   return true;
                               }
                               return false;
                           },
                           [this] {});

    machine_.addState(kStateNavigation,
                      [this] {
                          LOG_INFO("Entering %s", kStateNavigation);
                      },
                      [this] {
                          if (rx_original_goal().available())
                          {
                              auto original_goal = rx_original_goal().getProto();
                              auto goal = tx_goal().initProto();
                              // publish back the original goal
                              goal.setGoal(original_goal.getGoal());
                              goal.setGoalFrame(original_goal.getGoalFrame());
                              goal.setStopRobot(original_goal.getStopRobot());
                              goal.setTolerance(original_goal.getTolerance());
                              tx_goal().publish();
                          }
                      },
                      [] {});

    machine_.addTransition(kStateNavigation, kStateDetected,
                           [this] {
                               if (pixy.ccc.numBlocks && !success)
                               {
                                   return true;
                               }
                               return false;
                           },
                           [this] {
                           });

    machine_.addState(kStateDetected, [this] { 
        LOG_INFO("Entering %s", kStateDetected); 
        translation = false;
        rotation = false;
        prev_translation = false;
        prev_rotation = false;
    },
                      [this] {
        if (pixy.ccc.numBlocks) {
            Pose2d move_to = Pose2d::Translation(0,0);
            translation = false;
            rotation = false;
            Block block = pixy.ccc.blocks[0];
            if(std::abs(block.m_x-pixy.frameWidth/2) > get_center_threshold()*pixy.frameWidth){                
                target_centered = false;
                if(block.m_x - pixy.frameWidth/2 < 0) {
                    move_to = Pose2d::Rotation(M_PI_4)*Pose2d::Translation(0.2,0);
                } else {
                    move_to = Pose2d::Rotation(-M_PI_4)*Pose2d::Translation(0.2,0);
                }
                translation=false;
                rotation=true;
            } else {
                double area_detected = block.m_width * block.m_height;
                double pixyFrameArea = (pixy.frameWidth * pixy.frameHeight);
                if (area_detected / pixyFrameArea > get_area_threshold())
                {
                    move_to = Pose2d::Translation(0, 0);
                    translation=true;
                    prev_translation=true;
                    rotation=false;
                    shoot_target = true;
                    navigation_mode_->async_set_desired_behavior("stop");
                }
                else
                {
                    move_to = Pose2d::Translation(0.5, 0);
                    translation=true;
                    rotation=false;
                }                
            }

            if((rotation && !prev_rotation) || (translation && !prev_translation)){
                prev_rotation = rotation;
                prev_translation = translation;
                auto goal = tx_goal().initProto();
                Pose2d pose = get_world_T_robot(getTickTime()) * move_to;
                ToProto(pose, goal.initGoal());
                goal.setGoalFrame("world");
                goal.setStopRobot(false);
                goal.setTolerance(0.1);
                tx_goal().publish(); 
            }
        } else {
            LOG_ERROR("Lost lock!");
            translation = false;
            rotation = false;
            prev_translation = false;
            prev_rotation = false;
        } }, [] {});

    machine_.addTransition(kStateDetected, kStateShoot,
                           [this] {
                               if (shoot_target)
                               {
                                   return true;
                               }
                               return false;
                           },
                           [this] {
                           });

    machine_.addState(kStateShoot, [this] {
        LOG_INFO("Entering %s", kStateShoot);
        success=true;
        shoot_target=false;
        target_centered=false; 

        pixy.m_link.stop();
        int result;
        uint8_t *bayerFrame;
        uint32_t rgbFrame[PIXY2_RAW_FRAME_WIDTH * PIXY2_RAW_FRAME_HEIGHT];
        // grab raw frame, BGGR Bayer format, 1 byte per pixel
        pixy.m_link.getRawFrame(&bayerFrame);
        // convert Bayer frame to RGB frame
        demosaic(PIXY2_RAW_FRAME_WIDTH, PIXY2_RAW_FRAME_HEIGHT, bayerFrame, rgbFrame);
        // write frame to PPM file for verification
        result = writePPM(PIXY2_RAW_FRAME_WIDTH, PIXY2_RAW_FRAME_HEIGHT, rgbFrame, "out", ++index_frame);
        if(result < 0) {
        LOG_ERROR("Can't write PPM file");
        }
        pixy.m_link.resume();

    }, [] {}, [] {});

    machine_.addTransition(kStateShoot, kStateNavigation,
                           [this] {
                               if (success)
                               {
                                   return true;
                               }
                               return false;
                           },
                           [this] {
                           });

    machine_.addState(kStateExit, [this] {}, [] {}, [] {});
}
int writePPM(uint16_t width, uint16_t height, uint32_t *image, const char *filename, uint32_t index)
{
  int i, j;
  char fn[32];

  sprintf(fn, "/tmp/%s%010d.ppm", filename, index);
  FILE *fp = fopen(fn, "wb");
  if (fp == NULL)
    return -1;
  fprintf(fp, "P6\n%d %d\n255\n", width, height);
  for (j = 0; j < height; j++)
  {
    for (i = 0; i < width; i++)
      fwrite((char *)(image + j * width + i), 1, 3, fp);
  }
  fclose(fp);
  return 0;
}

void demosaic(uint16_t width, uint16_t height, const uint8_t *bayerImage, uint32_t *image)
{
  uint32_t x, y, r, g, b;
  int32_t xx, yy;
  uint8_t *pixel0, *pixel;

  for (y = 0; y < height; y++)
  {
    yy = y;
    if (yy == 0)
      yy++;
    else if (yy == height - 1)
      yy--;
    pixel0 = (uint8_t *)bayerImage + yy * width;
    for (x = 0; x < width; x++, image++)
    {
      xx = x;
      if (xx == 0)
        xx++;
      else if (xx == width - 1)
        xx--;
      pixel = pixel0 + xx;
      if (yy & 1)
      {
        if (xx & 1)
        {
          r = *pixel;
          g = (*(pixel - 1) + *(pixel + 1) + *(pixel + width) + *(pixel - width)) >> 2;
          b = (*(pixel - width - 1) + *(pixel - width + 1) + *(pixel + width - 1) + *(pixel + width + 1)) >> 2;
        }
        else
        {
          r = (*(pixel - 1) + *(pixel + 1)) >> 1;
          g = *pixel;
          b = (*(pixel - width) + *(pixel + width)) >> 1;
        }
      }
      else
      {
        if (xx & 1)
        {
          r = (*(pixel - width) + *(pixel + width)) >> 1;
          g = *pixel;
          b = (*(pixel - 1) + *(pixel + 1)) >> 1;
        }
        else
        {
          r = (*(pixel - width - 1) + *(pixel - width + 1) + *(pixel + width - 1) + *(pixel + width + 1)) >> 2;
          g = (*(pixel - 1) + *(pixel + 1) + *(pixel + width) + *(pixel - width)) >> 2;
          b = *pixel;
        }
      }
      *image = (b << 16) | (g << 8) | r;
    }
  }
}

} // namespace ev3
} // namespace isaac
