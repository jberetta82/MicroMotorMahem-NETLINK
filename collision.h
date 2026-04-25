#ifndef __Collision_H__
# define __Collision_H__

///COLLISION V4.8///

/*
***ATTRIBUTES***
0 = solid wall/floor
1 = ramp up
2 = ramp right
3 = ramp down
4 = ramp left
8 = rotating platform
9 = kill player
10 = Checkpoint 0 (start)
11 = Checkpoint 1
12 = Checkpoint 2
13 = Checkpoint 3
20 = spring
21 = ramp jump
100 - 199 = waypoint

*/

typedef struct {
	
    Sint16	 cen_x;	//x pos of coll cube		
	Sint16  cen_y;			
	Sint16  cen_z;	
	Uint16	 x_size; //x size of coll cube
	Uint16	 y_size;
	Uint16	 z_size;	
	Uint8  att;	//attribute	
	Sint8 rot;//car angle for ramp
	
	//int	 gru;	//group (model)	
} COLLISON;

typedef struct {
	COLLISON	*cotbl;		/* ポリゴン定義テーブル  collision definition table*/
	Uint8		 nbCo;		/* ポリゴンの数 number of coll cubes*/
}CDATA;







typedef struct section_data{
    //  Render flag
    //bool alive;
	
	//type
	Uint8 type;
	//bool door_open;
	//bool open_door;
	//bool close_door;
	//collisions
	
    // Position
    Sint16 x;
    Sint16 y;
    Sint16 z;
	
	//position adj (open doors)
	Sint16 tx;
	Sint16 ty;
	Sint16 tz;

    // Rotation
    //int rx;
    //int ry;
    //int rz;
	
	// Size (Hitboxes)
	//int xsize;
	//int	ysize;
	//int zsize;
	
	//model
	XPDATA	*map_model;
	PDATA	*map_model_lp;
	//collisions
	CDATA		*a_cdata;
	COLLISON	*a_collison;
	
	//centre of map adjustment
	//int map_adjust;
	//int map_yadjust;
	
	//mesh
	

   
   
}level_section;

level_section		map_section[];

//	AABB collision
Uint16 has_collision(Uint8 collpoints_type, Sint16 collpoints_x, Sint16 collpoints_y, Sint16 collpoints_z, Uint16 collpoints_xsize, Uint16 collpoints_ysize, Uint16 collpoints_zsize, Sint16 object_x, Sint16 object_y, Sint16 object_z, Uint16 object_xsize, Uint16 object_ysize, Uint16 object_zsize);

Uint16 has_horizontal_collision(Uint8 collpoints_type, Sint16 collpoints_x, Sint16 collpoints_y, Sint16 collpoints_z, Uint16 collpoints_xsize, Uint16 collpoints_ysize, Uint16 collpoints_zsize, Sint16 object_x, Sint16 object_y, Sint16 object_z, Uint16 object_xsize, Uint16 object_ysize, Uint16 object_zsize);

Uint16 has_vertical_collision(Uint8 collpoints_type, Sint16 collpoints_x, Sint16 collpoints_y, Sint16 collpoints_z, Uint16 collpoints_xsize, Uint16 collpoints_ysize, Uint16 collpoints_zsize, Sint16 object_x, Sint16 object_y, Sint16 object_z, Uint16 object_xsize, Uint16 object_ysize, Uint16 object_zsize);

Sint16 has_ramp_collision(Uint8 collpoints_type, Sint16 collpoints_x, Sint16 collpoints_y, Sint16 collpoints_z, Uint16 collpoints_xsize, Uint16 collpoints_ysize, Uint16 collpoints_zsize,Sint16 object_x, Sint16 object_y, Sint16 object_z, Uint16 object_xsize, Uint16 object_ysize, Uint16 object_zsize);

Uint8 has_trigger_collision(Uint8 collpoints_type, Sint16 collpoints_x, Sint16 collpoints_y, Sint16 collpoints_z, Uint16 collpoints_xsize, Uint16 collpoints_ysize, Uint16 collpoints_zsize, Sint16 object_x, Sint16 object_y, Sint16 object_z, Uint16 object_xsize, Uint16 object_ysize, Uint16 object_zsize);

Sint16 has_shadow_collision(Sint16 collpoints_x, Sint16 collpoints_y, Sint16 collpoints_z, Uint16 collpoints_xsize, Uint16 collpoints_ysize, Uint16 collpoints_zsize, Sint16 object_x, Sint16 object_y, Sint16 object_z, Uint16 object_xsize, Uint16 object_ysize, Uint16 object_zsize);

float has_ceiling_collision(Sint16 collpoints_x, Sint16 collpoints_y, Sint16 collpoints_z, Uint16 collpoints_xsize, Uint16 collpoints_ysize, Uint16 collpoints_zsize, Sint16 object_x, Sint16 object_y, Sint16 object_z, Uint16 object_xsize, Uint16 object_ysize, Uint16 object_zsize, float object_speed_y);

bool has_object_collision(Sint16 object1_x, Sint16 object1_y, Sint16 object1_z, Uint16 object1_xsize, Uint16 object1_ysize, Uint16 object1_zsize,
							Sint16 object2_x, Sint16 object2_y, Sint16 object2_z, Uint16 object2_xsize, Uint16 object2_ysize, Uint16 object2_zsize);

#endif /* !__Collision_H__ */
