
///COLLISION V4.8///

#include <jo/jo.h>
//#include "hamster.h"

//# define COLL_DIST              		(720)//512
#define NO_RAMP_COLLISION		(0)
#define CEILING_COLLISION		(999999)

float Lerp(float a, float b,float t)
{
	return a + t * (b - a);
	
}

//	AABB collision
Uint16 has_collision(Uint8 collpoints_type, Sint16 collpoints_x, Sint16 collpoints_y, Sint16 collpoints_z, Uint16 collpoints_xsize, Uint16 collpoints_ysize, Uint16 collpoints_zsize, Sint16 object_x, Sint16 object_y, Sint16 object_z, Uint16 object_xsize, Uint16 object_ysize, Uint16 object_zsize)
{
	Uint16 x_dist;
	Uint16 y_dist;
	Uint16 z_dist;
				
			
			x_dist = JO_ABS(object_x - collpoints_x);
			y_dist = JO_ABS(object_y - collpoints_y);
			z_dist = JO_ABS(object_z - collpoints_z);
					
			if(x_dist - (object_xsize + collpoints_xsize) >0 || y_dist - (object_ysize + collpoints_ysize) >0 || z_dist - (object_zsize + collpoints_zsize) >0)
			{
				return false;
			}			
			
			return true;
}

//	check X or Z collision
Uint16 has_horizontal_collision(Uint8 collpoints_type, Sint16 collpoints_x, Sint16 collpoints_y, Sint16 collpoints_z, Uint16 collpoints_xsize, Uint16 collpoints_ysize, Uint16 collpoints_zsize, Sint16 object_x, Sint16 object_y, Sint16 object_z, Uint16 object_xsize, Uint16 object_ysize, Uint16 object_zsize)
{
	Uint16 x_dist;
	Uint16 y_dist;
	Uint16 z_dist;
	Uint16 step;			
			
			x_dist = JO_ABS(object_x - collpoints_x);
			y_dist = JO_ABS(object_y - collpoints_y);
			z_dist = JO_ABS(object_z - collpoints_z);
				
			step = JO_ABS((collpoints_y - collpoints_ysize) - (object_y + object_ysize));
				
			if(x_dist - (object_xsize + collpoints_xsize) >0 || y_dist - (object_ysize + collpoints_ysize) >0 || z_dist - (object_zsize + collpoints_zsize) >0)
			{
				return false;
			}			
					
			//	Return if colliding			
			if(collpoints_type == 0)
			{
				return step;
			}
		
	return false;
}

Uint16 has_vertical_collision(Uint8 collpoints_type, Sint16 collpoints_x, Sint16 collpoints_y, Sint16 collpoints_z, Uint16 collpoints_xsize, Uint16 collpoints_ysize, Uint16 collpoints_zsize, Sint16 object_x, Sint16 object_y, Sint16 object_z, Uint16 object_xsize, Uint16 object_ysize, Uint16 object_zsize)
{

	Uint16 x_dist;
	Uint16 y_dist;
	Uint16 z_dist;
	Uint16 object_int_height;

							
			x_dist = JO_ABS(object_x - collpoints_x);
			y_dist = JO_ABS(object_y - collpoints_y);
			z_dist = JO_ABS(object_z - collpoints_z);
			
			if(x_dist - (object_xsize + collpoints_xsize) >0 || y_dist - (object_ysize + collpoints_ysize) >0 || z_dist - (object_zsize + collpoints_zsize) >0)
			{
				return NO_RAMP_COLLISION;
			}
			
			//	Return if colliding
			if(collpoints_type < 8)
			{					
				object_int_height = collpoints_y - (collpoints_ysize + object_ysize );
					
				return object_int_height == NO_RAMP_COLLISION ? 1 : object_int_height;
			}
			
	return NO_RAMP_COLLISION;
	
}



Sint16 has_ramp_collision(Uint8 collpoints_type, Sint16 collpoints_x, Sint16 collpoints_y, Sint16 collpoints_z, Uint16 collpoints_xsize, Uint16 collpoints_ysize, Uint16 collpoints_zsize,Sint16 object_x, Sint16 object_y, Sint16 object_z, Uint16 object_xsize, Uint16 object_ysize, Uint16 object_zsize)
{
	Uint16 x_dist;
	Uint16 y_dist;
	Uint16 z_dist;
	
	Sint16 object_R_int_height;	
	
	//set distance
	x_dist = JO_ABS(object_x - collpoints_x);
	y_dist = JO_ABS(object_y - collpoints_y);
	z_dist = JO_ABS(object_z - collpoints_z);
	
		
		if(collpoints_type < 1 || collpoints_type >4)
		{
		return NO_RAMP_COLLISION;
		}				
		x_dist = JO_ABS(object_x - collpoints_x);
		y_dist = JO_ABS((object_y + object_ysize) - collpoints_y);
		z_dist = JO_ABS(object_z - collpoints_z);
		
		if(x_dist - (object_xsize + collpoints_xsize) >0 || y_dist - (object_ysize + collpoints_ysize) >0 || z_dist - (object_zsize + collpoints_zsize) >0)
			{
				return false;
			}
			
		//	Return if colliding
		
			float bottom_height = 0;
			float top_height = (float) (collpoints_ysize *2);
			float location =0;
			float min_x = 0;
			float max_x = 0;
			float min_z = 0;
			float max_z = 0;
			
			switch(collpoints_type)
			{
			case 1: min_z = collpoints_z - collpoints_zsize;
					max_z = collpoints_z + collpoints_zsize;
					location = 1+((object_z - max_z)/(max_z - min_z));
					break;
				
			case 2: min_x = collpoints_x - collpoints_xsize;
					max_x = collpoints_x + collpoints_xsize;
					location = 1+((object_x - max_x)/(max_x - min_x));
					break;	
				
			case 3: min_z = collpoints_z + collpoints_zsize;
					max_z = collpoints_z - collpoints_zsize;
					location = 1+((object_z - max_z)/(max_z - min_z));
					break;		
					
			case 4: min_x = collpoints_x + collpoints_xsize;
					max_x = collpoints_x - collpoints_xsize;
					location = 1+((object_x - max_x)/(max_x - min_x));
					break;		
								
			}		
			
			object_R_int_height = object_ysize -(collpoints_y + collpoints_ysize) + Lerp(bottom_height,top_height,location);//(height at bottom, height at top, location between bottom and top 
			
			return object_R_int_height;		
	
}

//	check trigger collision, eg door
Uint8 has_trigger_collision(Uint8 collpoints_type, Sint16 collpoints_x, Sint16 collpoints_y, Sint16 collpoints_z, Uint16 collpoints_xsize, Uint16 collpoints_ysize, Uint16 collpoints_zsize, Sint16 object_x, Sint16 object_y, Sint16 object_z, Uint16 object_xsize, Uint16 object_ysize, Uint16 object_zsize)
{

	Uint16 x_dist;
	Uint16 y_dist;
	Uint16 z_dist;
						 
	if(collpoints_type > 7)
		{				
			x_dist = JO_ABS(object_x - collpoints_x);
			y_dist = JO_ABS(object_y - collpoints_y);
			z_dist = JO_ABS(object_z - collpoints_z);
				
			if(x_dist - (object_xsize + collpoints_xsize) >0 || y_dist - (object_ysize + collpoints_ysize) >0 || z_dist - (object_zsize + collpoints_zsize) >0)
			{
				return false;
			}	
								
			return collpoints_type;
				
		}
	
	return NO_RAMP_COLLISION;
}

Sint16 has_shadow_collision(Sint16 collpoints_x, Sint16 collpoints_y, Sint16 collpoints_z, Uint16 collpoints_xsize, Uint16 collpoints_ysize, Uint16 collpoints_zsize, Sint16 object_x, Sint16 object_y, Sint16 object_z, Uint16 object_xsize, Uint16 object_ysize, Uint16 object_zsize)
{
	Uint16 x_dist;
	Uint16 y_dist;
	Uint16 z_dist;
					
			x_dist = JO_ABS(object_x - collpoints_x);
			y_dist = JO_ABS(object_y - collpoints_y);
			z_dist = JO_ABS(object_z - collpoints_z);
						
			if(x_dist - (object_xsize + collpoints_xsize) >0 || y_dist - (object_ysize + collpoints_ysize) >0 || z_dist - (object_zsize + collpoints_zsize) >0)
			{
				return NO_RAMP_COLLISION;
			}
			
			//	Return if colliding
			return y_dist - collpoints_ysize;			

	
}

float has_ceiling_collision(Sint16 collpoints_x, Sint16 collpoints_y, Sint16 collpoints_z, Uint16 collpoints_xsize, Uint16 collpoints_ysize, Uint16 collpoints_zsize, Sint16 object_x, Sint16 object_y, Sint16 object_z, Uint16 object_xsize, Uint16 object_ysize, Uint16 object_zsize, float object_speed_y)
{
	
	Uint16 x_dist;
	Uint16 y_dist;
	Uint16 z_dist;
			
			x_dist = JO_ABS(object_x - collpoints_x);
			y_dist = JO_ABS((object_y - object_ysize) - collpoints_y);
			z_dist = JO_ABS(object_z - collpoints_z);
						
			if(x_dist - (object_xsize + collpoints_xsize) >0 || y_dist - (object_ysize + collpoints_ysize) >0 || z_dist - (object_zsize + collpoints_zsize) >0)
			{
				return false;
			}
			
			//	Return if colliding
			if(object_speed_y <=0)
			{
			return true;
			}
			
	return false;
	
}

bool has_object_collision(Sint16 object1_x, Sint16 object1_y, Sint16 object1_z, Uint16 object1_xsize, Uint16 object1_ysize, Uint16 object1_zsize,
							Sint16 object2_x, Sint16 object2_y, Sint16 object2_z, Uint16 object2_xsize, Uint16 object2_ysize, Uint16 object2_zsize)
{

	Uint16 x_dist;
	Uint16 y_dist;
	Uint16 z_dist;
		
	//set distance
	x_dist = JO_ABS(object1_x - object2_x);
	y_dist = JO_ABS(object1_y - object2_y);
	z_dist = JO_ABS(object1_z - object2_z);
			
		if(x_dist - (object1_xsize + object2_xsize) >0 || y_dist - (object1_ysize + object2_ysize) >0 || z_dist - (object1_zsize + object2_zsize) >0)
			{
				return false;
			}		
				
		//	Return if colliding
		return true;

	
	
}

