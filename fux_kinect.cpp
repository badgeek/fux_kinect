////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyright (c) 1997-1999 Mark Danks.
//    Copyright (c) G�nther Geiger.
//    Copyright (c) 2001-2002 IOhannes m zmoelnig. forum::f�r::uml�ute. IEM
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "fux_kinect.h"

#include "Base/GemMan.h"
#include "Base/GemCache.h"

#include <stdio.h>
#include <assert.h>

CPPEXTERN_NEW_WITH_GIMME(fux_kinect)

/////////////////////////////////////////////////////////
//
// fux_kinect
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////

static pthread_cond_t  gl_frame_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t gl_backbuf_mutex = PTHREAD_MUTEX_INITIALIZER; 

uint8_t *rgb_back, *rgb_mid, *rgb_front;
uint16_t t_gamma[2048];
uint8_t *depth_mid, *depth_front;

int got_rgb;
int got_depth;

int kinect_max;
int kinect_min;

//int global_depth[307200];

fux_kinect :: fux_kinect(int argc, t_atom *argv)
    	  : m_originalImage(NULL)
{

  inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("kinect_angle"));
  inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("kinect_max"));
  inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("kinect_min"));


  m_pixBlock.image = m_imageStruct;
  m_pixBlock.image.data = NULL;
  m_pixBlock.image.setCsizeByFormat(GL_LUMINANCE);

  m_width=640;
  m_height=480;
  kinect_video_size = m_width * m_height;

  requested_format = FREENECT_VIDEO_RGB;
  current_format   = FREENECT_VIDEO_RGB;
  kinect_angle     = 0;
 
  //create mutex  
  //gl_backbuf_mutex = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
  //if(&gl_backbuf_mutex){
   // if ( pthread_mutex_init(&gl_backbuf_mutex, NULL) < 0 ) {
      //free(&gl_backbuf_mutex);
      //&gl_backbuf_mutex=NULL;
    //} 
  //}

  pthread_mutex_init(&gl_backbuf_mutex, NULL);

 // gl_frame_cond = (pthread_cond_t*) malloc(sizeof(pthread_mutex_t));
  
  pthread_cond_init(&gl_frame_cond, NULL);


  if (freenect_init(&f_ctx, NULL) < 0) {
  	post("freenect_init() failed\n");
  }else{
  	post("kinect initiated\n");
  }
  
  int nr_devices = freenect_num_devices (f_ctx);
  post ("Number of devices found: %d\n", nr_devices);
  
  if (freenect_open_device(f_ctx, &f_dev, 0) < 0) {
  	post("Could not open device\n");
  }else{	
	int res = pthread_create(&freenect_thread, NULL, freenect_thread_func, this);
	  if (res) {
	  	post("pthread_create failed\n");
	  }
  }

  depth_mid = (uint8_t*)malloc(640*480);
  depth_front = (uint8_t*)malloc(640*480);
  //rgb_back = (uint8_t*)malloc(640*480*3);
  //rgb_mid = (uint8_t*)malloc(640*480*3);
  //rgb_front = (uint8_t*)malloc(640*480*3);

  got_rgb = 0;
  got_depth = 0;

  //depth map representation
  int i;
  for (i=0; i<2048; i++) {
  	float v = i/2047.0;
  	v = powf(v, 3)* 6;
  	t_gamma[i] = v*6*256;
  }

  banged = false;
  kinect_multiply = 10;
  kinect_resolution = 1;
  kinect_max = 2048;
  kinect_min = 0;
}



void *fux_kinect::freenect_thread_func(void*target)
{
	fux_kinect *me = (fux_kinect*) target;
	
	freenect_set_led(me->f_dev,LED_GREEN);
	freenect_set_depth_callback(me->f_dev, me->depth_cb);
	//freenect_set_video_callback(me->f_dev, me->rgb_cb);
	//freenect_set_video_mode(me->f_dev, freenect_find_video_mode(FREENECT_RESOLUTION_MEDIUM, me->current_format));
	freenect_set_depth_mode(me->f_dev, freenect_find_depth_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_DEPTH_11BIT));
	//freenect_set_video_buffer(me->f_dev, rgb_back);
	freenect_start_depth(me->f_dev);
	//freenect_start_video(me->f_dev);
		
	int accelCount = 0;

	while (freenect_process_events(me->f_ctx) >= 0) {
		//Throttle the text output
		/*if (accelCount++ >= 2000)
		{
			accelCount = 0;
			freenect_raw_tilt_state* state;
			freenect_update_tilt_state(me->f_dev);
			state = freenect_get_tilt_state(me->f_dev);
			double dx,dy,dz;
			freenect_get_mks_accel(state, &dx, &dy, &dz);
			me->post("raw acceleration: %4d %4d %4d  mks acceleration: %4f %4f %4f\n", state->accelerometer_x, state->accelerometer_y, state->accelerometer_z, dx, dy, dz);
		}*/

		if (me->requested_format != me->current_format) {
			//freenect_stop_video(me->f_dev);
			//freenect_set_video_mode(me->f_dev, freenect_find_video_mode(FREENECT_RESOLUTION_MEDIUM, me->requested_format));
			//freenect_start_video(me->f_dev);
			me->current_format = me->requested_format;
		}
	}
	
	return 0; //penting*/
}

void fux_kinect::depth_cb(freenect_device *dev, void *v_depth, uint32_t timestamp)
{

	int i;
	uint16_t *depth = (uint16_t*)v_depth;

	pthread_mutex_lock(&gl_backbuf_mutex);
	
	int depth_range = kinect_max - kinect_min;


	for (i=0; i<640*480; i++) {		
		//global_depth[i] = depth[i];
		//depth_mid[i] = depth[i];
		
	   int pval;
	
	   if(depth[i] > (float)kinect_min)
	   {

		if (depth[i] > (float)kinect_max)
		{
		 pval = 0;
		}else{
		 //depth[i] - kinect_max
	         pval = (1 - (depth[i] - (float)kinect_min)/(float)depth_range) * 255.f;//ceil((1-(depth[i]/(float)kinect_max))*255.f);    
		}

	   }else{
	       pval = 0;    
	   }
	   //int lb = (0xFF) << 24 | (pval & 0xFF) << 16 | (pval & 0xFF) << 8 | (pval & 0xFF) << 0;;
	   depth_mid[i] = pval;		
	}
    
	got_depth++;
	pthread_cond_signal(&gl_frame_cond);
	pthread_mutex_unlock(&gl_backbuf_mutex);
}

void fux_kinect::rgb_cb(freenect_device *dev, void *rgb, uint32_t timestamp)
{
	//pthread_mutex_lock(gl_backbuf_mutex);

	// swap buffers
	//assert (rgb_back == rgb);
	//rgb_back = rgb_mid;
	//freenect_set_video_buffer(dev, rgb_back);
	//rgb_mid = (uint8_t*)rgb;

	//got_rgb++;
	//pthread_cond_signal(gl_frame_cond);
	//pthread_mutex_unlock(gl_backbuf_mutex);
}




/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
fux_kinect :: ~fux_kinect()
{
    cleanImage();
	freenect_stop_depth(f_dev);
	//freenect_stop_video(f_dev);
	freenect_close_device(f_dev);
	freenect_shutdown(f_ctx);
	free(depth_mid);
	post("shutdown kinect\n");
}


/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void fux_kinect :: render(GemState *state)
{
	pthread_mutex_lock(&gl_backbuf_mutex);
	// When using YUV_RGB mode, RGB frames only arrive at 15Hz, so we shouldn't force them to draw in lock-step.
	// However, this is CPU/GPU intensive when we are receiving frames in lockstep.
	//if (current_format == FREENECT_VIDEO_YUV_RGB) {
	//	while (!got_depth && !got_rgb) {
	//		pthread_cond_wait(gl_frame_cond, gl_backbuf_mutex);
	//	}
	//} else {
	//	while ((!got_depth || !got_rgb) && requested_format != current_format) {
	//		pthread_cond_wait(gl_frame_cond, gl_backbuf_mutex);
	//	}
	//}

	if (requested_format != current_format) {
		pthread_mutex_unlock(&gl_backbuf_mutex);
		return;
	}

	uint8_t *tmp;

	if (got_depth) {
		tmp = depth_front;
		depth_front = depth_mid;
		depth_mid = tmp;
		got_depth = 0;
	}
	
	//if (got_rgb) {
	//	tmp = rgb_front;
	//	rgb_front = rgb_mid;
	//	rgb_mid = tmp;
	//	got_rgb = 0;
	//}
		
	pthread_mutex_unlock(&gl_backbuf_mutex);
	
	unsigned char *pixels = m_pixBlock.image.data;
	
	uint8_t *depth_pixel = depth_front;
	int pval;
		
	for(int y = 0; y < kinect_video_size; y++) {
			pixels[y] = depth_pixel[y];	
	}
	
	m_pixBlock.newimage = 1;
	m_pixBlock.image.notowned = true;
	m_pixBlock.image.upsidedown = true;
	state->image = &m_pixBlock;
	
}

void fux_kinect :: startRendering(){	
  m_pixBlock.image.xsize = m_width;
  m_pixBlock.image.ysize = m_height;
  m_pixBlock.image.csize = 1; //GRAY
  m_pixBlock.image.reallocate();
  //m_rendering=true;
}

/////////////////////////////////////////////////////////
// postrender
//
/////////////////////////////////////////////////////////
void fux_kinect :: postrender(GemState *state)
{
    m_pixBlock.newimage = 0;
    state->image = NULL;
}


/////////////////////////////////////////////////////////
// kinectAngle
//
/////////////////////////////////////////////////////////
void fux_kinect :: kinectAngle(float gsize)
{
	 if (gsize >= -30 && gsize <= 30)
	 {
		 freenect_set_tilt_degs(f_dev,gsize);
	 }
}

void fux_kinect :: kinectMin(int gsize)
{
	kinect_min = gsize;
}

void fux_kinect :: kinectMax(int gsize)
{
	kinect_max = gsize;	
}
/////////////////////////////////////////////////////////
// cleanImage
//
/////////////////////////////////////////////////////////
void fux_kinect :: cleanImage()
{
    // release previous data
    if (m_originalImage)
    {
      delete m_originalImage;
      m_originalImage = NULL;
      m_pixBlock.image.clear();
    }
}


//void fux_kinect :: kinectResolution(float resolution)
//{
//	kinect_resolution = resolution;
//	post("kinect resolution...");
//}

/////////////////////////////////////////////////////////
// static member functions
//
/////////////////////////////////////////////////////////
void fux_kinect :: obj_setupCallback(t_class *classPtr)
{
    class_addmethod(classPtr, (t_method)&fux_kinect::kinectAngleCallback, gensym("kinect_angle"), A_FLOAT, A_NULL);
	class_addmethod(classPtr, (t_method)&fux_kinect::kinectMaxCallback, gensym("kinect_max"), A_FLOAT, A_NULL);
    class_addmethod(classPtr, (t_method)&fux_kinect::kinectMinCallback, gensym("kinect_min"), A_FLOAT, A_NULL);
}

void fux_kinect :: kinectAngleCallback(void *data, t_floatarg size)
{
    GetMyClass(data)->kinectAngle((float)size);
}

void fux_kinect :: kinectMaxCallback(void *data, t_floatarg size)
{
	GetMyClass(data)->kinectMax((int)size);
}

void fux_kinect :: kinectMinCallback(void *data, t_floatarg size)
{
	GetMyClass(data)->kinectMin((int)size);
}

