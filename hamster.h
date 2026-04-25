/*
** Jo Sega Saturn Engine
** Copyright (c) 2012-2017, Johannes Fetz (johannesfetz@gmail.com)
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**     * Redistributions of source code must retain the above copyright
**       notice, this list of conditions and the following disclaimer.
**     * Redistributions in binary form must reproduce the above copyright
**       notice, this list of conditions and the following disclaimer in the
**       documentation and/or other materials provided with the distribution.
**     * Neither the name of the Johannes Fetz nor the
**       names of its contributors may be used to endorse or promote products
**       derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
** DISCLAIMED. IN NO EVENT SHALL Johannes Fetz BE LIABLE FOR ANY
** DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
** (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
** LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
** ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
** SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _HAMSTER_H
# define _HAMSTER_H

# define WORLD_DEFAULT_X                (0)
# define WORLD_DEFAULT_Y                (-64)//-64
# define WORLD_DEFAULT_Z                (-64)


# define CAM_DEFAULT_X                (0)
# define CAM_DEFAULT_Y                (-24) //-64
# define CAM_ZOOM_1		              (50) 
# define CAM_ZOOM_2                    (100) 
# define CAM_ZOOM_3                   (200)
# define CAM_DEFAULT_Z                (-150)//-150
# define CAM_DIST_DEFAULT				(-24)
# define CAM_SPEED                		(10)//5
# define CAM_MAX_DIST              		(400)
# define COLL_DIST              		(900)//720
# define DRAW_DISTANCE					(1280)//512
# define DRAW_DISTANCE_2					(7000)//768
# define DRAW_DISTANCE_X				(300)
# define DRAW_DISTANCE_3					(300)
# define DRAW_DISTANCE_MAX					(5000)//768
#define FOV 						(DEGtoANG(80.0))

# define DOOR_SPEED					(2)
# define DOOR_RANGE					(8)

//define game states
#define GAMESTATE_UNINITIALIZED         (0)
#define GAMESTATE_TITLE_SCREEN          (1)
#define GAMESTATE_LEVEL_SELECT          (2)
#define GAMESTATE_GAMEPLAY              (3)
#define GAMESTATE_PAUSED                (4)
#define GAMESTATE_GAME_OVER             (5)
#define GAMESTATE_VICTORY               (6)
#define GAMESTATE_MAP_BUILDER           (7)
#define GAMESTATE_END_LEVEL				(8)
#define GAMESTATE_OBJECT_VIEWER			(9)
#define GAMESTATE_EXTRA_SELECT			(10)
#define GAMESTATE_PLAYER_SELECT			(11)
#define GAMESTATE_RACE_START			(12)

#define GAMEMODE_PRACTICE		       (0)
#define GAMEMODE_TIMEATTACK		       (1)
#define GAMEMODE_1PLAYERRACE           (2)
#define GAMEMODE_1PLAYERSURVIVAL       (4)
#define GAMEMODE_2PLAYERVS             (3)


//define menu options
#define PAUSE_MENU_MAX               (7)
#define LEVEL_MENU_MAX               (15)
#define SURVIVAL_MENU_MAX            (50)
#define END_LEVEL_MENU_MAX           (2)
#define TITLE_SCREEN_MENU_MAX		 (4)
#define EXTRA_MENU_MAX		 		 (3)
#define PLAYER_SELECT_MENU_MAX		 (3)

#define ANIM_SPEED              (3)
#define MAX_PLAYERS         	(2)
#define PLAYER_SPEED         	(2)
#define PLAYER_HURT_TIMER       (64)
#define PLAYER_STUCK_TIMER       (64)
#define PLAYER_CLOUD_TIMER      (20)
#define RACE_START_TIMER        (180)
#define RACE_END_TIMER			(120)
#define ENEMY_HURT_TIMER        (20)
#define ENEMY_JUMP_TIMER        (60)
#define PUP_TIMER       	 	(380)
#define NO_RAMP_COLLISION		(0)
#define CEILING_COLLISION		(999999)
#define MAX_LAPS				(3)
#define MAX_COLOUR				(12)
#define INV_SIZE				(28)

#define MAP_WIDTH 33
#define MAP_HEIGHT 33

int track_layout[MAP_WIDTH * MAP_HEIGHT] = {
    9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
    9,0,0,0,9,9,9,0,0,9,0,0,0,0,0,0,9,9,9,9,0,0,0,9,0,0,9,0,0,0,0,0,9,
    9,0,9,0,0,0,9,0,9,9,0,9,9,9,9,0,0,0,9,0,0,9,0,9,0,9,9,0,9,0,9,0,9,
    9,0,9,9,9,0,9,0,9,0,0,9,0,0,0,9,9,0,0,0,9,0,0,9,0,9,0,0,9,0,9,0,9,
    9,0,0,0,9,0,0,0,0,0,9,0,0,9,0,9,9,9,0,9,9,9,0,0,0,0,0,9,9,0,9,9,9,
    9,0,9,0,9,0,9,9,0,9,9,9,9,9,0,9,0,0,0,9,0,0,9,0,9,9,9,9,0,0,0,0,9,
    9,0,9,0,9,0,0,9,9,9,0,9,0,0,0,9,0,9,0,9,9,0,0,0,0,9,9,9,0,9,9,0,9,
    9,0,9,9,9,9,9,9,0,9,0,9,0,9,0,9,9,9,0,0,9,0,9,9,0,0,0,0,0,9,0,0,9,
    9,0,0,0,0,0,0,0,0,9,0,0,0,0,9,9,0,9,0,9,0,0,0,9,9,0,9,0,9,9,0,9,9,
    9,9,0,9,9,9,0,9,0,0,9,9,9,0,0,9,0,9,0,9,0,9,0,9,0,0,9,0,0,0,0,0,9,
    9,9,0,9,9,9,0,9,9,0,0,0,0,9,0,0,0,9,0,9,0,0,9,0,0,9,9,9,9,9,9,0,9,
    9,0,0,9,0,0,0,0,9,9,0,9,0,9,0,9,9,9,9,0,9,0,9,9,0,9,0,9,0,9,0,0,9,
    9,9,9,9,0,9,9,9,9,9,0,9,0,9,0,0,0,0,0,0,9,0,0,0,0,0,0,9,0,9,9,0,9,
    9,0,0,9,0,9,0,0,9,0,9,9,9,9,0,9,0,9,0,9,9,9,0,9,9,9,9,9,0,0,0,0,9,
    9,0,9,0,0,9,9,0,9,0,0,0,0,0,0,9,0,9,9,0,0,0,0,0,0,0,0,9,9,9,9,0,9,
    9,0,9,0,9,9,0,0,9,0,9,9,0,9,9,0,0,0,9,9,0,9,9,9,0,9,0,0,0,0,0,0,9,
    9,0,9,0,0,9,0,9,9,0,0,9,0,0,0,0,0,0,0,0,0,9,0,9,0,9,9,9,0,9,9,0,9,
    9,0,9,9,0,0,0,9,9,9,9,9,0,9,9,0,0,0,9,9,0,9,0,9,0,0,0,9,0,9,0,0,9,
    9,0,0,9,9,0,9,9,0,0,0,0,0,0,9,9,0,9,9,0,0,0,0,9,0,8,0,9,0,9,0,9,9,
    9,9,0,0,0,0,9,9,0,9,0,9,9,9,0,9,0,9,0,9,0,9,9,9,0,0,0,9,0,9,0,9,9,
    9,0,9,0,9,0,0,0,0,9,0,9,0,0,0,0,0,0,0,9,0,0,0,9,9,0,9,0,0,9,0,0,9,
    9,0,0,0,9,0,9,9,0,9,0,0,0,9,9,0,9,9,0,9,9,9,0,0,0,0,0,9,9,9,9,0,9,
    9,9,9,0,0,0,0,9,0,9,9,9,9,9,0,0,0,9,0,0,0,9,9,9,9,9,0,9,0,0,0,0,9,
    9,0,9,9,0,9,0,9,0,0,9,0,0,0,0,9,0,9,9,9,0,9,0,0,0,0,0,0,0,9,9,0,9,
    9,0,0,0,0,9,0,0,9,9,9,0,9,9,9,0,0,9,0,0,0,0,0,0,0,9,9,9,0,0,0,0,9,
    9,9,9,9,0,9,9,9,9,0,0,0,0,9,9,9,9,9,0,9,9,9,0,0,0,9,0,9,0,9,9,0,9,
    9,9,0,9,0,0,0,0,9,0,9,0,9,9,0,9,0,9,0,9,0,0,9,9,9,0,0,9,0,9,0,0,9,
    9,0,0,0,0,9,9,0,0,9,0,0,9,0,0,9,0,0,0,9,0,0,9,0,0,0,9,9,0,9,0,9,9,
    9,9,9,9,9,9,9,0,0,9,0,9,9,0,9,9,9,9,0,9,9,0,9,0,9,9,9,9,0,0,0,9,9,
    9,0,0,0,9,0,0,0,9,0,0,9,0,0,0,9,0,9,0,0,0,0,9,0,9,0,0,9,0,9,0,0,9,
    9,0,1,0,9,0,9,0,9,0,9,9,0,9,0,9,0,9,9,0,9,9,9,0,9,9,0,9,9,9,9,0,9,
    9,0,0,0,0,0,9,0,0,0,9,9,0,9,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9,
    9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9
};

char *car_name[]={

"DEFAULT",	
"FORMULA 1",
"LAMBO",
"JEEEP",
"COP CAR",
"CHARGER",
"RACING",
"FIREBIRD",

};

char *survival_preview[]={
	
"SWB.TGA",

};

int survival_target_time[]={
	
30

};

int points_table[]={
	
0,
4,
3,
2,
1

};

typedef struct	_CHECKPOINT
{
	Uint16				section;
    Sint16				x;
	Sint16				y;
	Sint16				z;
	Sint16				ry;
    
}       

                checkpoint;
				
typedef struct	_WAYPOINT
{
	Uint16				section;
    Sint16				x;
	Sint16				y;
	Sint16				z;
    
}       

                waypoint;
				
typedef struct	_CLOUD
{
    Sint16				x;
	Sint16				y;
	Sint16				z;
	Sint16				ry;
	Uint8				size;
    
}       

                cloud;
				
typedef struct	
{
	char 				level_name[20];
	Uint16				level_target_time;
	char				level_preview[12];
	char				level_map[12];
	char				tileset[12];
	char 				file_name[12];
	char				floor[12];
	char				sky[12];
	Uint8				is_inside;
	Uint8				cd_track;
	char 				difficulty[20];
	Uint16				level_fastest_lap;


    
}       
LDATA;


				
LDATA level_data[] = {
   {"TITLE", 0, "SBT.TGA", "MBT2.TGA", "TITLE.TGA", "TITLE.BIN", "FLR1.TGA", "SKY8.TGA", 0, 2, "NA",0},
   {"BREAKFAST 1", 80, "SBT.TGA", "MBT1.TGA", "BT.TGA", "BT1.BIN", "FLR5.TGA", "SKY8.TGA", 1, 3, "EASY",0},
   {"DESERT 1", 80, "SDS.TGA", "MDS1.TGA", "DS.TGA", "DS1.BIN", "SAND.TGA", "SKY6.TGA", 0, 4, "EASY",0},
   {"WORKBENCH 1", 80, "SWB.TGA", "MWB1.TGA", "WB.TGA", "WB1.BIN", "FLR3.TGA", "SKY7.TGA", 1, 5, "MEDIUM",0},
   {"POOLTABLE 1", 85, "SPT.TGA", "MPT1.TGA", "PT.TGA", "PT1.BIN", "FLR1.TGA", "SKY8.TGA", 1, 7, "MEDIUM",0},
   {"RUINS 1", 85, "RUP.TGA", "MRU1.TGA", "RU.TGA", "RU1.BIN", "WATER.TGA", "SKY8.TGA", 0, 6, "MEDIUM",0}, 
   {"BREAKFAST 2", 80, "SBT.TGA", "MBT2.TGA", "BT.TGA", "BT2.BIN", "FLR5.TGA", "SKY8.TGA", 1, 3, "MEDIUM",0},
   {"DESERT 2", 90, "SDS.TGA", "MDS2.TGA", "DS.TGA", "DS2.BIN", "SAND.TGA", "SKY6.TGA", 0, 4, "MEDIUM",0},
   {"RUINS 2", 135, "RU2.TGA", "MRU2.TGA", "RU.TGA", "RU3.BIN", "WATER.TGA", "SKY8.TGA", 0, 6, "MEDIUM",0},
   {"POOLTABLE 4", 210, "SPT.TGA", "MPT4.TGA", "PT.TGA", "PT4.BIN", "FLR1.TGA", "SKY8.TGA", 1, 7, "MEDIUM",0},
   {"WORKBENCH 2", 85, "SWB.TGA", "MWB2.TGA", "WB.TGA", "WB2.BIN", "FLR3.TGA", "SKY7.TGA", 1, 5, "HARD",0},
   {"BREAKFAST 4", 120, "SBT.TGA", "MBT4.TGA", "BT.TGA", "BT4.BIN", "FLR5.TGA", "SKY8.TGA", 1, 3, "MEDIUM",0},
   {"DESERT 3", 90, "SDS.TGA", "MDS3.TGA", "DS.TGA", "DS3.BIN", "SAND.TGA", "SKY6.TGA", 0, 4, "MEDIUM",0},
   {"DESKTOP 1", 150, "SDK.TGA", "MDT1.TGA", "DT.TGA", "DT1.BIN", "FLR6.TGA", "SKY8.TGA", 1, 5, "MEDIUM",0}, 
   {"RUINS 3", 85, "STW.TGA", "MRU3.TGA", "RU.TGA", "RU2.BIN", "WATER.TGA", "SKY8.TGA", 0, 6, "HARD",0},   
   {"CC TOILET", 85, "SCC.TGA", "MCC1.TGA", "CC.TGA", "CC1.BIN", "FLR3.TGA", "SKY8.TGA", 0, 8, "EASY",0},  
   
  
  
   
   
   
   // {"DINNERTABLE", 80, "SDT.TGA", "BT2.TGA", "BT3.BIN", "FLR5.TGA", "SKY8.TGA", 1, 0, "MEDIUM"},
};
				
Uint16 colour_options[13][3] = {
{0,157,255},
{251,92,4},
{255,0,108},
{126,255,0},
{213,218,104},
{222,161,163},
{255,159,219},
{43,77,134},
{255,214,0},
{0,0,0},
{255,255,255},
{64,0,64},
{202,6,6}

};

Uint16 shadow_colour[13][3] = {
{26,94,136},
{197,77,10},
{184,8,83},
{96,187,7},
{147,150,79},
{166,129,130},
{179,126,159},
{29,44,69},
{181,155,21},
{98,98,98},
{176,176,176},
{31,1,42},
{124,17,17}

};
// set default car colours for 4 players
Uint8 default_colour[8][12] = {
{0,1,0, 2,3,2, 3,2,3, 8,0,8},
{1,0,1, 12,10,12, 7,8,7, 9,10,9},
{4,5,4, 1,0,1, 3,2,3, 8,7,8},
{2,0,2, 3,2,3, 1,3,1, 0,2,0},
{10,11,10, 10,11,10, 10,11,10, 10,11,10},
{7,6,7, 1,0,1, 8,12,8, 1,11,1},
{12,10,12, 7,10,7, 8,10,8, 11,10,11},
{9,8,9, 8,9,8, 3,2,3, 8,0,8}
};

char *cam_modes[]={
	
"FIXED",
"FOLLOW",
"S FOLLOW"
};

char *menu_titlescreen[]={
	
"PRACTICE",
"TIME ATTACK",
"SINGLE RACE",
"2 PLAYER VS"
};

char *menu_extra[]={
	
"MAP VIEWER",
"CAR VIEWER",
"BACK"
};

char *menu_pause[]={
	
"CONTINUE",
"RESET TO LAST CHECKPOINT",
"RESTART LEVEL",
"MUSIC VOL",
"CAMERA",
"LEVEL MAP",
"QUIT"
};

char *menu_endlevel[]={
	
"CONTINUE",
"QUIT"
};

char *menu_endlevelwin[]={
	
"CONTINUE TO NEXT LEVEL",
"QUIT"
};

char *menu_endlevellose[]={
	
"RETRY",
"QUIT"
};
typedef struct  _PLAYER
{
	Uint16 colour1;
	Uint16 colour2;
	Uint16 colour3;
	Uint16 gstart;
	bool alive;
	Uint8 state;
	Uint8 wins;
	Uint8 health;
	Uint8 points;
	Uint8 total_points;
	Uint8 position;
	Uint8 total_position;
	bool hurt;
	cloud clouds[5];
	bool can_make_clouds;
	Sint8 cloud_number;
	Sint16 cloud_timer;
	cloud particles[5];
	bool can_make_particles;
	Sint8 particle_number;
	Sint16 particle_timer;
	Uint8 type;
	Sint16 hud_sprite;
	bool can_be_hurt;
	Sint16 hurt_timer;
	Sint16 pup_timer;
	Sint16 stuck_timer;
	bool mutate;
	Uint8 current_powerup;
	bool powerup_active;
	Uint8 powerup_id;
	Sint8 gamepad;
	bool pressed_start;
	bool pressed_up;
	bool pressed_down;
	Sint8 dpad;
	Sint16 gems;
	bool on_ladder_x;	
	bool on_ladder_z;
	bool on_ceiling;
	bool in_water;
	bool flapping;
	Uint8 car_selection;
	bool car_selected;
	   
	// Start Position
    Sint16 start_x;
    Sint16 start_y;
    Sint16 start_z;
	
	// Position
    Sint16 x;
    Sint16 y;
    Sint16 z;
	Sint16 nextx;
	Sint16 nexty;
	Sint16 nextz;
	float delta_x;
	float delta_z;	

    // Rotation
    Sint16 rx;
    Sint16 ry;
    Sint16 rz;
	Sint16 ary;
	
	// Physics
	bool    physics_is_in_air;
    float   physics_air_acceleration_strength;
    float   physics_acceleration_strength;
    float   physics_friction;
	float   physics_friction2;
    float   physics_deceleration_strength;
    float   physics_max_speed;
    FIXED   physics_gravity;
	FIXED	physics_wgravity;
    FIXED   physics_jump_speed_y;
    float   physics_speed;
    FIXED   physics_speed_y;
	float   physics_speed_x_adj;
	float   physics_speed_z_adj;
	float   physics_speed_x;
	float   physics_speed_z;
	float 	physics_turn_speed;
	float 	physics_grip;
	bool	drift;
	float 	physics_boost_max_speed;
	float	physics_boost_acceleration_strength;
	float 	physics_small_max_speed;
	float	object_scale;
	
	
	//cpu control
	bool cpu_left;
	bool cpu_right;
	bool cpu_gas;
	bool cpu_brake;
	bool cpu_action;
	bool cpu_pressed_action;
	bool cpu_siren;
	Uint8 next_waypoint;
	Uint8 current_waypoint;
	Sint16 tx;
	Sint16 ty;
	Sint16 tz;
	Sint16 tr;
	bool enable_controls;
	Sint16 dist_to_next_waypoint;
	Sint16 position_points;
	
	//wheel Rotation
	Sint16 wheel_rx;
	Sint16 wheel_ry;
			
	//shadow
	Sint16 shadow_y;
	float shadow_size;
	Sint16 current_shadow_map_section;
	
	Sint16 anim_speed;
	
	// Size (Hitboxes)
    Sint16 xsize;
    Sint16 ysize;
    Sint16 zsize;
	
	Uint16 current_map_section;
	Uint16 current_collision;
	Uint16 current_shadow_collision;
	bool can_jump;
	//float jump_height;
	
	//projectile is active
	bool projectile_alive;
	
	//projectile
	Sint16 px;
	Sint16 py;
	Sint16 pz;
	Sint16 pr;
	Sint16 bomb_speed;
	Sint16 projectile_speed;
	Sint16 speed_px;
	Sint16 speed_py;
	Sint16 speed_pz;
	
	//projectile target
	Sint16 ptx;
	Sint16 pty;
	Sint16 ptz;
	
	//bool aim;
	bool shoot;
	bool action;
	
	Uint16 r;
	Uint16 g;
	Uint16 b;
	
	Sint16 effect_x;
	Sint16 effect_y;
	Sint16 effect_z;
	float effect_size;
	Uint8 effect_type;
	Sint16 effect_ry;
	
	Sint16 explosion_size;
	
	//lap time
	Uint8 laps;
	Uint16 start_time;
	Uint16 current_time;
	Uint16 best_time;
	Uint16 mins;	
	Uint16 secs;
	Uint8 current_checkpoint;
	Uint16 total_time;
	
	//camera
	
	Sint16	cam_pos_x;
	Sint16	cam_pos_y;
	Sint16	cam_pos_z;
	Sint16	cam_target_x;
	Sint16	cam_target_y;
	Sint16	cam_target_z;
	Sint16	cam_angle_x;
	Sint16	cam_angle_y;
	Sint16	cam_angle_z;
	Sint16	cam_horiz_dist;
	Sint16	cam_vert_dist;
	Sint16	cam_pitch;
	Sint16	cam_pitch_adj;
	Sint16	cam_angle_adj;
	Sint16 cam_dist;
	Sint16 cam_height;
	Uint8 cam_zoom_num;
	Sint16 cam_zoom;
	bool	rear_cam;
	
	//soundfx
	short eng1_sound;
	short eng2_sound;
	short eng3_sound;
	short eng4_sound;
	short eng5_sound;
	short eng6_sound;
	short eng7_sound;
	short horn_sound;
	short drift_sound;
	short siren_sound;
	Uint8 volume;
	
	Uint8 player_select_menu;
	Uint16 		pause_time;
	
	//end race
	bool		race_ended;

}               player_params;

extern player_params players[4];




static  __jo_force_inline void            physics_accelerate_backwards(int p)
{
    players[p].physics_speed -= (players[p].physics_is_in_air ? players[p].physics_air_acceleration_strength : players[p].physics_acceleration_strength);
    players[p].physics_speed = JO_MAX(players[p].physics_speed, JO_CHANGE_SIGN(players[p].physics_max_speed));
}

static  __jo_force_inline void            physics_accelerate_forwards(int p)
{
    players[p].physics_speed += (players[p].physics_is_in_air ? players[p].physics_air_acceleration_strength : players[p].physics_acceleration_strength);
    players[p].physics_speed = JO_MIN(players[p].physics_speed, players[p].physics_max_speed);
}

static  __jo_force_inline void            physics_boost(int p)
{
    players[p].physics_speed += (players[p].physics_is_in_air ? players[p].physics_air_acceleration_strength : players[p].physics_boost_acceleration_strength);
    players[p].physics_speed = JO_MIN(players[p].physics_speed, players[p].physics_boost_max_speed);
}

static  __jo_force_inline void            physics_accelerate_forwards_small(int p)
{
    players[p].physics_speed += (players[p].physics_is_in_air ? players[p].physics_air_acceleration_strength : players[p].physics_acceleration_strength);
    players[p].physics_speed = JO_MIN(players[p].physics_speed, players[p].physics_small_max_speed);
}

static  __jo_force_inline void            physics_decelerate_backwards(int p)
{
    players[p].physics_speed +=  players[p].physics_deceleration_strength;
    players[p].physics_speed = JO_MIN(players[p].physics_speed, 0);
}

static  __jo_force_inline void            physics_decelerate_forwards(int p)
{
    players[p].physics_speed -=  players[p].physics_deceleration_strength;
    players[p].physics_speed = JO_MAX(0, players[p].physics_speed);
}

static  __jo_force_inline void              physics_apply_friction(int p)
{
    players[p].physics_speed -= JO_MIN(JO_ABS(players[p].physics_speed), players[p].physics_friction) * (players[p].physics_speed > 0 ? 1 : -1);
}

static  __jo_force_inline void              physics_apply_friction_x(int p)
{
    players[p].physics_speed_x_adj -= JO_MIN(JO_ABS(players[p].physics_speed_x_adj), players[p].physics_friction2) * (players[p].physics_speed_x_adj > 0 ? 1 : -1);
}

static  __jo_force_inline void              physics_apply_friction_z(int p)
{
    players[p].physics_speed_z_adj -= JO_MIN(JO_ABS(players[p].physics_speed_z_adj), players[p].physics_friction2) * (players[p].physics_speed_z_adj > 0 ? 1 : -1);
}

typedef struct	_ENEMY
{
	bool alive;
	Sint16 health;
	Sint16 start_health;
	bool hurt;
	Uint8 type;
	Sint16 hud_sprite;
	bool can_be_hurt;
	Sint16 hurt_timer;
	Sint16 anim_speed;
	Sint16 death_timer;

    // Start Position
    Sint16 start_x;
    Sint16 start_y;
    Sint16 start_z;
		
	// Position
	Sint16 x;
    Sint16 y;
    Sint16 z;
	Sint16 max_speed;
	Sint16 speed_x;
	float speed_y;
	Sint16 speed_z;
	Sint16 xdist;
	Sint16 zdist;
	//bool flip_direction;
	Sint16 waypoint;
	
	// Size
    Sint16 xsize;
    Sint16 ysize;
    Sint16 zsize;
	
	//air bubbles
	float air_bubble_size;
	Sint16 air_bubble_x;
	float air_bubble_y;
	Sint16 air_bubble_z;
	

    // Rotation
    Sint16 rx;
    float ry;
    Sint16 rz;
	
	//head Rotation
	Sint16 head_rx;
	Sint16 head_ry;
	Sint16 head_rz;
	
	//body Rotation
	Sint16 body_rx;
	Sint16 body_ry;
	Sint16 body_rz;
	
	//arm Rotation
	Sint16 larm_rx;
	Sint16 larm_ry;
	Sint16 larm_rz;
	Sint16 rarm_rx;
	Sint16 rarm_ry;
	Sint16 rarm_rz;
	
	//leg Rotation
	Sint16 lleg_rx;
	Sint16 lleg_ry;
	Sint16 lleg_rz;
	Sint16 rleg_rx;
	Sint16 rleg_ry;
	Sint16 rleg_rz;
	
	//target
	Sint16 target;
	Sint16 tx;
	Sint16 ty;
	Sint16 tz;
	float tr;
	
	//melee attack
	Sint16 attack_timer;
	bool attack; 
	
	//jump
	Sint16 jump_timer;
	
	//die effect
	float effect_size;
	
	
	
	//projectile is active
	bool projectile_alive;
	Sint16 shoot_counter;
	Sint16 shoot_wait;
	bool can_shoot;
	
	//projectile position
	Sint16	px;
	Sint16 py;
	Sint16	pz;
	
	float speed_px;
	float speed_py;
	float speed_pz;
	
	//projectile target
	Sint16 ptx;
	Sint16 pty;
	Sint16 ptz;
	
	//projectile direction
	float pr;
	
	
	//ramp
	float R_int_height;
	float int_height;
	Sint16 ramp_height_adj;
	Sint16 current_map_section;
	bool can_jump;
	
	//death
	Sint16 explosion_size;
	

	
	

	
}				enemy;
extern enemy 	enemies[];

typedef struct	_POWERUP
{

	Uint8			type;
	Sint16			x;
	Sint16			y;
	Sint16			z;
	Sint16			ry;
	bool		used;
	bool		used_saved;
	Uint8			lev;
	//model
	XPDATA	*pup_model;
	

	
}				powerup;
extern powerup	powerups[];

typedef struct  _GAME
{
    // game state variables
	Sint16 		map_sprite_id;
	Sint16		hud_sprite_id;
	Sint16 		p1_back_sprite_id;
	Sint16 		p1_shadow_sprite_id;
	Sint16 		p2_back_sprite_id;
	Sint16 		p2_shadow_sprite_id;
	Sint16 		effect_sprite_id_1;
    Uint8         game_state;
	Uint8         mode;
	Uint8		extra_menu;
    Uint8         pause_menu;
    bool        pressed_start;
	bool        pressed_left;
	bool        pressed_right;
    bool        pressed_up;
    bool        pressed_down;
	bool        pressed_X;
	Uint8		players;
	Uint16		level_1_gems;
	Uint8		end_level_menu;
	Uint8 		level_select_menu;
	Uint8 		title_screen_menu;
	Uint8		select_level;
	Uint8		level;
	Uint16 		pause_time;
	Uint16		pause_start_time;
	Sint16		start_x;
	Sint16		start_z;
	Uint16 		target_mins;	
	Uint16 		target_secs;
	Uint16 		target_time;
	Sint16		race_start_timer;
	Sint16		race_end_timer;
	Sint16		frame_counter;
	Uint8		winner;
	bool		enable_shadows;
	bool		level_inside;
	Uint8		finishing_position;
	bool		show_total_table;


}               game_params;
extern game_params game;



#endif /* !_HAMSTER_H */

/*
** END OF FILE
*/
