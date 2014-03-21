/*

   Demonstrate graphics bitmap compositing.

*/

#include "pebble.h"
#include "time.h"

static Window *window;

static Layer* window_layer;

static BitmapLayer *layer;

static GBitmap *off_image;

static GBitmap *on_image;

static GBitmap *timeMasks;

static inline GColor bmpGetPixel(const GBitmap *bmp, int x, int y) {
    if (x >= bmp->bounds.size.w || y >= bmp->bounds.size.h || x < 0 || y < 0) return -1;
    int byteoffset = y*bmp->row_size_bytes + x/8;
    return ((((uint8_t *)bmp->addr)[byteoffset] & (1<<(x%8))) != 0);
}

static void layer_update_callback(Layer *layer, GContext* ctx) {
  time_t rawtime;
  time( &rawtime );
  struct tm* now = localtime(&rawtime);
  int h = ((now->tm_hour%12)==0)?12:(now->tm_hour%12);
  int m = now->tm_min;
  graphics_draw_bitmap_in_rect(ctx, off_image, off_image->bounds);
  for (int i = 0; i < 12; i += 1) {
    for (int j = 0; j < 12; j += 1) {
      if(bmpGetPixel(timeMasks, (i*12)+j, ((h - 1) * 12) + (m / 5))==GColorWhite){
        GBitmap* sub_bmp = gbitmap_create_as_sub_bitmap(on_image, (GRect) { .origin = { j * 12, (i * 14) }, .size = { 12, 16 } });
        graphics_draw_bitmap_in_rect(ctx, sub_bmp, (GRect) { .origin = { j * 12, (i * 14) }, .size = { 12, 16 } });
        gbitmap_destroy(sub_bmp);
      }
    }
  }
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed){
  int multiple_of_5 = tick_time->tm_min%5==0;
  if(multiple_of_5){  
    layer_mark_dirty((Layer *)layer);
  }
}

int main(void) {

  // Then use the respective resource loader to obtain the resource for use
  // In this case, we load the image
  off_image = gbitmap_create_with_resource(RESOURCE_ID_OFF_RESOURCE);
  on_image = gbitmap_create_with_resource(RESOURCE_ID_ON_RESOURCE);
  timeMasks = gbitmap_create_with_resource(RESOURCE_ID_TIME_MASKS);
  window = window_create();
  window_stack_push(window, true /* Animated */);

  // Initialize the layer
  window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);
  layer = bitmap_layer_create(bounds);
  bitmap_layer_set_alignment(layer, GAlignTopLeft);
  bitmap_layer_set_background_color(layer, GColorWhite);

  // Set up the update layer callback
  layer_set_update_proc((Layer *)layer, layer_update_callback);

  // Add the layer to the window for display
  layer_add_child(window_layer, (Layer *)layer);
  

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  layer_mark_dirty((Layer *)layer);
 
  // Enter the main loop
  app_event_loop();

  
  // Cleanup the image
  gbitmap_destroy(on_image);
  gbitmap_destroy(off_image);
  gbitmap_destroy(timeMasks);

  layer_destroy((Layer *)layer);
  
  window_destroy(window);
}
