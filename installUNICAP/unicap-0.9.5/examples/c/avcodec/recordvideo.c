/*
** recordvideo.c
** 
** Made by (Arne Caspari)
** Login   <arne@arne-laptop>
** 
** Started on  Sun Oct 22 19:40:21 2006 Arne Caspari
** Last update Fri Feb 16 17:10:29 2007 Arne Caspari
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <signal.h>

#include <unicap.h>
#include <ucil.h>

#ifdef HAVE_AV_CONFIG_H
#undef HAVE_AV_CONFIG_H
#endif

#include <avcodec.h>

int PixelFormatBPP[] = {
   //PIX_FMT_NONE= -1,
   12,// PIX_FMT_YUV420P,   ///< Planar YUV 4:2:0, 12bpp, (1 Cr & Cb sample per 2x2 Y samples)
   16,// PIX_FMT_YUYV422,   ///< Packed YUV 4:2:2, 16bpp, Y0 Cb Y1 Cr
   24,// PIX_FMT_RGB24,     ///< Packed RGB 8:8:8, 24bpp, RGBRGB...
   24,// PIX_FMT_BGR24,     ///< Packed RGB 8:8:8, 24bpp, BGRBGR...
   16,// PIX_FMT_YUV422P,   ///< Planar YUV 4:2:2, 16bpp, (1 Cr & Cb sample per 2x1 Y samples)
   24,// PIX_FMT_YUV444P,   ///< Planar YUV 4:4:4, 24bpp, (1 Cr & Cb sample per 1x1 Y samples)
   32,// PIX_FMT_RGB32,     ///< Packed RGB 8:8:8, 32bpp, (msb)8A 8R 8G 8B(lsb), in cpu endianness
   9,// PIX_FMT_YUV410P,   ///< Planar YUV 4:1:0,  9bpp, (1 Cr & Cb sample per 4x4 Y samples)
   12,// PIX_FMT_YUV411P,   ///< Planar YUV 4:1:1, 12bpp, (1 Cr & Cb sample per 4x1 Y samples)
   16,// PIX_FMT_RGB565,    ///< Packed RGB 5:6:5, 16bpp, (msb)   5R 6G 5B(lsb), in cpu endianness
   16,// PIX_FMT_RGB555,    ///< Packed RGB 5:5:5, 16bpp, (msb)1A 5R 5G 5B(lsb), in cpu endianness most significant bit to 1
   8,// PIX_FMT_GRAY8,     ///<        Y        ,  8bpp
   1,// PIX_FMT_MONOWHITE, ///<        Y        ,  1bpp, 1 is white
   1,// PIX_FMT_MONOBLACK, ///<        Y        ,  1bpp, 0 is black
   8,// PIX_FMT_PAL8,      ///< 8 bit with PIX_FMT_RGB32 palette
   12,// PIX_FMT_YUVJ420P,  ///< Planar YUV 4:2:0, 12bpp, full scale (jpeg)
   16,// PIX_FMT_YUVJ422P,  ///< Planar YUV 4:2:2, 16bpp, full scale (jpeg)
   24,// PIX_FMT_YUVJ444P,  ///< Planar YUV 4:4:4, 24bpp, full scale (jpeg)
   8,// PIX_FMT_XVMC_MPEG2_MC,///< XVideo Motion Acceleration via common packet passing(xvmc_render.h)
   8,// PIX_FMT_XVMC_MPEG2_IDCT,
   16,// PIX_FMT_UYVY422,   ///< Packed YUV 4:2:2, 16bpp, Cb Y0 Cr Y1
   12,// PIX_FMT_UYYVYY411, ///< Packed YUV 4:1:1, 12bpp, Cb Y0 Y1 Cr Y2 Y3
   // PIX_FMT_NB,        ///< number of pixel formats, DO NOT USE THIS if you want to link with shared libav* because the number of formats might differ between versions
};

typedef struct PixelFormatFourccMap
{
      unsigned int fourcc;
      int pix_fmt;
} PixelFormatFourccMap;


struct PixelFormatFourccMap PixelFormatFourccs[] = 
{
   { UCIL_FOURCC( 'Y','U','Y','V' ), PIX_FMT_YUYV422 },
   { UCIL_FOURCC( 'R','G','B','3' ), PIX_FMT_RGB24 },
   { UCIL_FOURCC( 'R','G','B','4' ), PIX_FMT_RGB32 },
   { UCIL_FOURCC( 'U','Y','V','Y' ), PIX_FMT_UYVY422 },
   { UCIL_FOURCC( 'Y','8','0','0' ), PIX_FMT_GRAY8 },
};

static int num_pix_fmt_fourccs =  sizeof( PixelFormatFourccs ) / sizeof( PixelFormatFourccMap );


int outbuf_size;
uint8_t *outbuf = 0;
unicap_data_buffer_t recbuffer;
#define FOURCC(a,b,c,d) (unsigned int)((((unsigned int)a))+(((unsigned int)b)<<8)+(((unsigned int)c)<<16)+(((unsigned int)d)<<24))

struct timeval starttime;
struct timeval endtime;

volatile int started = 0;
volatile int framenum = 0;
volatile int quit = 0;

struct new_frame_data
{
      FILE *f;
      AVCodecContext *context;
      AVFrame *picture;
      int pix_fmt;
};

typedef struct new_frame_data new_frame_data_t;


void sighandler(int sig )
{
   quit = 1;
}



int convert_image( unicap_data_buffer_t *data_buffer, AVFrame *frame, int dest_pix_fmt )
{
   int src_pix_fmt = -1;
   int i;
   AVPicture src;
   AVPicture dest;
   int dest_bpp;
   
   for( i = 0; ( i < num_pix_fmt_fourccs ) && ( src_pix_fmt == -1 ); i++ )
   {
      if( PixelFormatFourccs[i].fourcc == data_buffer->format.fourcc )
      {
	 src_pix_fmt = PixelFormatFourccs[i].pix_fmt;
      }
   }
   
   if( src_pix_fmt == -1 )
   {
      printf( "src_pix_fmt == -1 fourcc= %08x %c%c%c%c\n", data_buffer->format.fourcc, (data_buffer->format.fourcc>>24) & 0xff,
	      (data_buffer->format.fourcc>>16) & 0xff,(data_buffer->format.fourcc>>8) & 0xff,(data_buffer->format.fourcc) & 0xff);
      return 0;
   }
   
   src.data[0] = src.data[1] = src.data[2] = src.data[3] = data_buffer->data;
   // reassign src->data[x] here for packet formats
   src.linesize[0] = src.linesize[1] = src.linesize[2] = src.linesize[3] = data_buffer->format.size.width * data_buffer->format.bpp / 8;

   if( ( dest_pix_fmt < 0 ) || ( dest_pix_fmt > PIX_FMT_NB ) )
   {
      printf( "dest pix fmt out of range\n" );
      return 0;
   }
   avpicture_fill( (AVPicture*)&dest, frame->data[0], dest_pix_fmt, data_buffer->format.size.width, data_buffer->format.size.height );
   
   img_convert( &dest, dest_pix_fmt, &src, src_pix_fmt, data_buffer->format.size.width, data_buffer->format.size.height );
   
   
   
   return 1;
}

   
   


AVCodecContext *setup_codec( new_frame_data_t *data, unicap_format_t *format )
{
   AVCodec *codec;
   AVCodecContext *c;

   codec = avcodec_find_encoder( CODEC_ID_H263P );
/*    codec = avcodec_find_encoder( CODEC_ID_MPEG2VIDEO ); */
   if( !codec )
   {
      fprintf( stderr, "Codec not found!\n" );
      return NULL;
   }
   
   c = avcodec_alloc_context( );
   c->bit_rate = 3000000;
   c->width = format->size.width;
   c->height = format->size.height;
   c->time_base = (AVRational){1,30};
   c->pix_fmt = PIX_FMT_YUV420P;

   if( avcodec_open( c, codec ) < 0 )
   {
      fprintf( stderr, "Could not open codec\n" );
      return NULL;
   }
   
   data->pix_fmt = c->pix_fmt;
   
   return c;
}


void encode_frame( FILE *f, AVCodecContext *c, AVFrame *picture )
{
   int out_size;
   
   out_size = avcodec_encode_video( c, outbuf, outbuf_size, picture );
   
   fwrite( outbuf, 1, out_size, f );
}

AVFrame *allocate_avframe( int width, int height, int pix_fmt )
{
   int bpp;
   int size;
   AVFrame *frame;
   unsigned char *framebuffer;
   
   
   if( ( pix_fmt < 0 ) || ( pix_fmt > PIX_FMT_NB ) )
   {
      return NULL;
   }
   
   bpp = PixelFormatBPP[pix_fmt];
   size = width * height * bpp / 8;
   
   frame = avcodec_alloc_frame( );
   framebuffer = malloc( avpicture_get_size( pix_fmt, width, height) );
   avpicture_fill( (AVPicture*)frame, framebuffer, pix_fmt, width, height );
   
   return frame;
}


static void new_frame_cb( unicap_event_t event, unicap_handle_t handle, unicap_data_buffer_t *buffer, new_frame_data_t *data )
{
   if( !started )
   {
      started = 1;
      gettimeofday( &starttime, NULL );
   }
   else
   {
      gettimeofday( &endtime, NULL );
   }

   framenum++;
   
   convert_image( buffer, data->picture, data->pix_fmt );
   
   encode_frame( data->f, data->context, data->picture );
}


void finish_encode( FILE *f, AVCodecContext *c )
{
   int out_size;
   
   for( out_size = 1; out_size; out_size = avcodec_encode_video( c, outbuf, outbuf_size, NULL ) )
   {
      fwrite( outbuf, 1, out_size, f );
   }

   outbuf[0] = 0x0;
   outbuf[1] = 0x0;
   outbuf[2] = 0x1;
   outbuf[3] = 0xb7;
   
   fwrite( outbuf, 1, 4, f );
}


void printstats(void)
{
   endtime.tv_sec -= starttime.tv_sec;
   endtime.tv_usec += endtime.tv_sec * 1000000;
   
   printf( "%d frames in %d usecs = %f FPS\n", framenum, endtime.tv_usec, (double)(framenum*1000000)/(double)(endtime.tv_usec));
}


int main( int argc, char **argv )
{
   unicap_handle_t handle;
   unicap_device_t device;
   unicap_format_t format_spec;
   unicap_format_t format;
   unicap_data_buffer_t buffer;
   int i;
   FILE *f;
   AVCodecContext *c;
   AVFrame *picture;

   new_frame_data_t new_frame_data;
   

   outbuf_size = 1000000;
   outbuf = malloc( outbuf_size );

   avcodec_init();
   avcodec_register_all();

   /*
     Enumerate available video capture devices
    */
   printf( "select video device\n" );
   for( i = 0; SUCCESS( unicap_enumerate_devices( NULL, &device, i ) ); i++ )
   {
      printf( "%i: %s\n", i, device.identifier );
   }
   if( --i > 0 )
   {
      printf( "Select video capture device: " );
      scanf( "%d", &i );
   }

   if( !SUCCESS( unicap_enumerate_devices( NULL, &device, i ) ) )
   {
      fprintf( stderr, "Failed to get info for device '%s'\n", device.identifier );
      exit( 1 );
   }

   /*
     Acquire a handle to selected device
   */
   if( !SUCCESS( unicap_open( &handle, &device ) ) )
   {
      fprintf( stderr, "Failed to open device: %s\n", device.identifier );
      exit( 1 );
   }

   printf( "Opened video capture device: %s\n", device.identifier );

   unicap_void_format( &format_spec );
	
   /*
     Get the list of video formats
   */
   for( i = 0; SUCCESS( unicap_enumerate_formats( handle, 
						  NULL, 
						  &format, i ) ); 
	i++ )
   {
      printf( "%d: %s\n", 
	      i,
	      format.identifier );
   }
   if( --i > 0 )
   {
      printf( "Select video format: " );
      scanf( "%d", &i );
   }
   if( !SUCCESS( unicap_enumerate_formats( handle, &format_spec, &format, i ) ) )
   {
      fprintf( stderr, "Failed to get video format\n" );
      exit( 1 );
   }
	
   /*
     If a video format has more than one size, ask for which size to use
   */
   if( format.size_count )
   {
      for( i = 0; i < format.size_count; i++ )
      {
	 printf( "%d: %dx%d\n", i, format.sizes[i].width, format.sizes[i].height );
      }
      do
      {
	 printf( "Select video format size: " );
	 scanf( "%d", &i );
      }while( ( i < 0 ) && ( i > format.size_count ) );
      format.size.width = format.sizes[i].width;
      format.size.height = format.sizes[i].height;
   }

   format.buffer_type = UNICAP_BUFFER_TYPE_SYSTEM;
   /*
     Set this video format
   */
   if( !SUCCESS( unicap_set_format( handle, &format ) ) )
   {
      fprintf( stderr, "Failed to set video format\n" );
      exit( 1 );
   }

   /*
     Initialize the image buffer
   */
   memset( &buffer, 0x0, sizeof( unicap_data_buffer_t ) );
   buffer.buffer_size = format.size.width * format.size.height * format.bpp / 8;
   buffer.data = malloc( format.buffer_size );
   memcpy( &recbuffer.format, &format, sizeof( unicap_format_t ) );
   recbuffer.format.fourcc = FOURCC( 'I', '4', '2', '0' );
   recbuffer.data = malloc( format.size.width * format.size.height * 2 );
   

   f = fopen( "out.mpeg", "wb" );
   c = setup_codec( &new_frame_data, &format );
   if( !c )
   {
      exit( -1 );
   }
   
   
   picture = allocate_avframe( format.size.width, format.size.height, new_frame_data.pix_fmt );

   new_frame_data.f = f;
   new_frame_data.context = c;
   new_frame_data.picture = picture;

   unicap_register_callback( handle, UNICAP_EVENT_NEW_FRAME, (unicap_callback_t)new_frame_cb, &new_frame_data );   
   
   /*
     Start the capture process on the device
   */
   if( !SUCCESS( unicap_start_capture( handle ) ) )
   {
      fprintf( stderr, "Failed to start capture on device: %s\n", device.identifier );
      exit( 1 );
   }
   
   signal( SIGINT, sighandler );   

   while( !quit )
   {
      usleep( 100000 );
   }

   unicap_stop_capture( handle );
   
   printf( "finish..\n" );

   finish_encode( f, c );
   
   fclose( f );

   printstats();
   

   return 0;
   
}
