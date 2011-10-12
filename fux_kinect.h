/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    Snap a pix of the render buffer

    Copyright (c) 1997-1999 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2002 IOhannes m zmoelnig. forum::für::umläute. IEM. zmoelnig@iem.kug.ac.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef INCLUDE_PIX_SNAP_H_
#define INCLUDE_PIX_SNAP_H_

#include "Base/GemBase.h"
#include "Base/GemPixUtil.h"
#include "FreeImage.h"
#include "libfreenect.h"

#include <pthread.h>

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    fux_kinect
    
    Snaps a pix of the render buffer
    
KEYWORDS
    pix
    
DESCRIPTION

    Inlet for a list - "vert_size"
    Inlet for a list - "vert_pos"

    "snap" - Snap a pix
    "vert_size" - Set the size of the pix
    "vert_pos" - Set the position of the pix
    
-----------------------------------------------------------------*/
class GEM_EXTERN fux_kinect : public GemBase
{
    CPPEXTERN_HEADER(fux_kinect, GemBase)

    public:

        //////////
        // Constructor
    	fux_kinect(int argc, t_atom *argv);
		
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~fux_kinect();

    	//////////
	//	void kinectResolution(float resolution);
		
		// save point cloud
		//void saveKinectPoint();

		// multiply distance between point
		//void kinectMultiply(float size);
		

    	// When a angle is received
    	virtual void	kinectAngle(float gsize);
		virtual void    kinectMin(int gsize);
		virtual void    kinectMax(int gsize);

		///start rendering
		virtual void	startRendering();

    	//////////
    	// Do the rendering
    	virtual void 	render(GemState *state);

    	//////////
    	// Clear the dirty flag on the pixBlock
    	virtual void 	postrender(GemState *state);
    	
    	//////////
    	// Clean up the image
    	void	    	cleanImage();
    	
    	//////////
    	// The original fux_kinect
    	imageStruct 	*m_originalImage;
    	
    	//////////
    	// The pixBlock with the current image
    	pixBlock    	m_pixBlock;
		imageStruct     m_imageStruct;
    	
    	//////////
    	// The x position
    	int m_x;
    	
    	//////////
    	// The y position
    	int m_y;
    	
    	//////////
    	// The width
    	int m_width;
    	
    	//////////
    	// The height
    	int m_height;
		
		// 
		
		//Kinect
		static void* freenect_thread_func(void*);
		static void depth_cb(freenect_device *dev, void *v_depth, uint32_t timestamp);
		static void rgb_cb(freenect_device *dev, void *rgb, uint32_t timestamp);
		
		
		
    private:
    	
    //////////
    // static member functions
	static void kinectAngleCallback(void *data, t_floatarg size);	
	static void kinectMaxCallback(void *data, t_floatarg size);	
	static void kinectMinCallback(void *data, t_floatarg size);	
	
	freenect_context *f_ctx;
	freenect_device *f_dev;	

	freenect_video_format requested_format;
	freenect_video_format current_format;
		
	pthread_t freenect_thread;

	int kinect_angle;
	int kinect_video_size;
	int kinect_multiply;
	int kinect_resolution;
	
	
	bool banged;

};

#endif	// for header file
