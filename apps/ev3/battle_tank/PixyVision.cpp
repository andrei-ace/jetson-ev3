#include "PixyVision.hpp"

#include "engine/core/logger.hpp"

namespace isaac
{
namespace ev3
{

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

void PixyVision::start()
{
  tickPeriodically();
  int result = pixy.init();
  if (result < 0)
  {
    LOG_ERROR("Error");
    LOG_ERROR("pixy.init() returned %d", result);
  }
  else
  {
    LOG_INFO("Initialised Pixy2 Camera");
  }  
}

void PixyVision::tick()
{
  
  // {
  //   pixy.m_link.stop();
  //   int result;
  //   uint8_t *bayerFrame;
  //   uint32_t rgbFrame[PIXY2_RAW_FRAME_WIDTH * PIXY2_RAW_FRAME_HEIGHT];
  //   // grab raw frame, BGGR Bayer format, 1 byte per pixel
  //   pixy.m_link.getRawFrame(&bayerFrame);
  //   // convert Bayer frame to RGB frame
  //   demosaic(PIXY2_RAW_FRAME_WIDTH, PIXY2_RAW_FRAME_HEIGHT, bayerFrame, rgbFrame);
  //   // write frame to PPM file for verification
  //   result = writePPM(PIXY2_RAW_FRAME_WIDTH, PIXY2_RAW_FRAME_HEIGHT, rgbFrame, "out", ++index_frame);
  //   if(result < 0) {
  //     LOG_ERROR("Can't write PPM file");
  //   }
  //   pixy.m_link.resume();
  // }
  {
    pixy.ccc.getBlocks();

    if (pixy.ccc.numBlocks)
    {
      LOG_INFO("Detected %d block(s)", pixy.ccc.numBlocks);
      for (int block_index = 0; block_index < pixy.ccc.numBlocks; ++block_index)
      {
        LOG_INFO("Block %d", block_index + 1);
        pixy.ccc.blocks[block_index].print();
      }
    }
  }
}

void PixyVision::stop()
{
 
}

} // namespace ev3
} // namespace isaac
