
#include <stdint.h>
#include <string.h>
#include "png_file.h"

void png_file_init(const char* filename, png_file_t* png)
{
   png->file = fopen(filename, "rb");

   png->handle = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
   png->info = png_create_info_struct(png->handle);
   png_init_io(png->handle, png->file);

   png_read_png(png->handle, png->info, PNG_TRANSFORM_IDENTITY, NULL);

   png->width = png_get_image_width(png->handle, png->info);
   png->height = png_get_image_height(png->handle, png->info);
   png->rows = png_get_rows(png->handle, png->info);
}

void png_file_free(png_file_t* png)
{
   png_destroy_info_struct(png->handle, &png->info);
   png_destroy_read_struct(&png->handle, &png->info, NULL);
   png_free(png->handle, NULL);
   memset(png, 0, sizeof(*png));
}

void png_file_read(png_file_t* png, uint8_t* dst, int pitch)
{
   int i;
   for (i = 0; i < png->height; i++)
   {
      uint8_t* in = png->rows[i];
      uint32_t* out = (uint32_t*)(dst + i * pitch);
      int x;
      for(x = 0; x < png->width; x++)
      {
         uint8_t r = *(in++);
         uint8_t g = *(in++);
         uint8_t b = *(in++);
         *(out++) = (r << 0) | (g << 8) | (b << 16);
      }
   }
}
