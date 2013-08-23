/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "frei0r.h"

typedef struct bgsubtract0r_instance
{
  unsigned int width;
  unsigned int height;
  uint8_t threshold;
  char denoise; /* Remove noise from mask. */
  uint32_t fill; /* The color for filling background. */
  uint32_t* reference; /* The reference image. */
  unsigned char* mask; /* Where the mask is computed. */
} bgsubtract0r_instance_t;

int f0r_init()
{
  return 1;
}

void f0r_deinit()
{
}

void f0r_get_plugin_info(f0r_plugin_info_t* bgsubtract0r_info)
{
  bgsubtract0r_info->name = "BgSubtract0r";
  bgsubtract0r_info->author = "Samuel Mimram";
  bgsubtract0r_info->plugin_type = F0R_PLUGIN_TYPE_FILTER;
  bgsubtract0r_info->color_model = F0R_COLOR_MODEL_RGBA8888;
  bgsubtract0r_info->frei0r_version = FREI0R_MAJOR_VERSION;
  bgsubtract0r_info->major_version = 0;
  bgsubtract0r_info->minor_version = 2;
  bgsubtract0r_info->num_params =  3;
  bgsubtract0r_info->explanation = "Bluescreen the background of a static video.";
}

f0r_instance_t f0r_construct(unsigned int width, unsigned int height)
{
  bgsubtract0r_instance_t* inst = (bgsubtract0r_instance_t*)calloc(1, sizeof(*inst));
  inst->width = width;
  inst->height = height;
  inst->denoise = 1;
  inst->threshold = 26;
  inst->fill = 0x0000ff00;
  inst->reference = NULL;
  inst->mask = malloc(width*height);
  return (f0r_instance_t)inst;
}

void f0r_destruct(f0r_instance_t instance)
{
  bgsubtract0r_instance_t* inst = (bgsubtract0r_instance_t*)instance;
  if (inst->reference) free(inst->reference);
  free(inst->mask);
  free(inst);
}

void f0r_get_param_info(f0r_param_info_t* info, int param_index)
{
  switch(param_index)
  {
  case 0:
    info->name = "color";
    info->type = F0R_PARAM_COLOR;
    info->explanation = "Bluescreen color";
    break;

  case 1:
    info->name = "threshold";
    info->type = F0R_PARAM_DOUBLE;
    info->explanation = "Threshold for difference";
    break;

  case 2:
    info->name = "denoise";
    info->type = F0R_PARAM_BOOL;
    info->explanation = "Remove noise";
    break;
  }
}

void f0r_set_param_value(f0r_instance_t instance, f0r_param_t param, int param_index)
{
  assert(instance);
  bgsubtract0r_instance_t* inst = (bgsubtract0r_instance_t*)instance;
  f0r_param_color_t *c;

  switch(param_index)
  {
  case 0:
    c = (f0r_param_color_t*)param;
    inst->fill = ((uint32_t)c->b*255) + (((uint32_t)c->g*255)<<8) + (((uint32_t)c->r*255)<<16);
    break;

  case 1:
    inst->threshold = *((double*)param) * 255.;
    break;

  case 2:
    inst->denoise = *((double*)param) >= 0.5;
    break;
  }
}

void f0r_get_param_value(f0r_instance_t instance, f0r_param_t param, int param_index)
{
  assert(instance);
  bgsubtract0r_instance_t* inst = (bgsubtract0r_instance_t*)instance;
  f0r_param_color_t *c;

  switch(param_index)
  {
  case 0:
    c = (f0r_param_color_t*)param;
    c->r = (inst->fill&0xff) / 255.;
    c->g = (inst->fill>>8&0xff) / 255.;
    c->b = (inst->fill>>16&0xff) / 255.;
    break;

  case 1:
    *((double*)param) = (double)inst->threshold / 255.;
    break;

  case 2:
    *((double*)param) = inst->denoise ? 1. : 0.;
    break;
  }
}

#define MAX(x,y) ((x) > (y) ? (x) : (y));

inline static uint8_t dst(uint32_t x, uint32_t y)
{
  uint8_t d;

  d = abs((x&0xff)-(y&0xff));
  d = MAX(d, abs(((x>>8)&0xff)-((y>>8)&0xff)));
  d = MAX(d, abs(((x>>16)&0xff)-((y>>16)&0xff)));

  return d;
}

void f0r_update(f0r_instance_t instance, double time, const uint32_t* inframe, uint32_t* outframe)
{
  assert(instance);
  bgsubtract0r_instance_t* inst = (bgsubtract0r_instance_t*)instance;
  unsigned int width = inst->width;
  unsigned int height = inst->height;
  unsigned int len = width * height;
  unsigned char *mask = inst->mask;
  int i;
  int j;
  int n;

  if (!inst->reference)
  {
    int blen = sizeof(uint32_t)*len;
    inst->reference = malloc(blen);
    memmove(inst->reference, inframe, blen);
    memset(mask, 0, blen);
  }
  else
  {
    for (i=0; i<len; i++)
      mask[i] = (dst(inst->reference[i], inframe[i]) > inst->threshold);
  }

  /* Clean up the mask. */
  if (inst->denoise)
    for (j=1; j<height-1; j++)
    {
      for (i=1; i<width-1; i++)
      {
        n = mask[width*j+i-1]+mask[width*j+i+1]+mask[width*(j-1)+i]+mask[width*(j+1)+i]
          + mask[width*(j-1)+i-1]+mask[width*(j-1)+i+1]+mask[width*(j+1)+i-1]+mask[width*(j+1)+i+1];
        if (mask[width*j+i])
        {
          if (n<=2) mask[width*j+i] = 0;
        }
        else
        {
          if (n>=6) mask[width*j+i] = 1;
        }
      }
      /*
      // Left
      n = mask[width*(j-1)]+mask[width*(j-1)+1]+mask[width*j+1]+mask[width*(j+1)+1]+mask[width*(j+1)];
      if (mask[width*j+i])
      {
        if (n<=1) mask[width*j] = 0;
      }
      else
      {
        if (n>=4) mask[width*j] = 1;
      }
      // Right
      n = mask[width*j-1]+mask[width*j]+mask[width*(j+1)]+mask[width*(j+2)]+mask[width*(j+2)-1];
      if (mask[width*j+i])
      {
        if (n<=1) mask[width*(j+1)-1] = 0;
      }
      else
      {
        if (n>=4) mask[width*(j+1)-1] = 1;
      }
      */
    }

  for(i=0; i<len; i++)
  {
    if (mask[i])
      outframe[i] = inframe[i];
    else
    {
      outframe[i] = inst->fill;
      // Updating does not work very well...
      /* inst->reference[i] = inframe[i]; */
    }
  }
}
