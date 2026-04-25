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

/***************************************************************************************************\
** Special Note : The 3D part on Jo Engine is still in development. So, some glitch may occur ;)
**                Btw, texture mapping for triangle base mesh (not quads) is experimental.
\***************************************************************************************************/


#include <jo/jo.h>
#include "pcmsys.h"
#include "collision.h"
#include "objects.h"
#include "hamster.h"
#include "texture.h"

extern Sint8 SynchConst;
Sint32 framerate;

#define WORK_RAM_LOW 0x00200000

//Controls
#define KEY_PRESS(id, key)  ((Smpc_Peripheral[id].data & key) == 0)
#define KEY_DOWN(id, key)   ((Smpc_Peripheral[id].push & key) == 0)

player_params 		players[4];
static XPDATA *xpdata_[32];
static PDATA *pdata_LP_[32];
static CDATA *cdata_[32];
level_section		map_section[32];
enemy 				enemies[1];
powerup				powerups[8];
checkpoint			checkpoints[4];
waypoint			waypoints[100];
game_params 		game;

jo_pos3Df                   pos;
jo_rot3Df                   rot;
jo_palette                  sky_pal;
jo_palette                  floor_pal;
jo_palette                  preview_pal;
jo_palette					trackmap_pal;
jo_palette                  font_pal;

GOURAUDTBL		gourRealMax[GOUR_REAL_MAX];
Uint8			vwork[GOUR_REAL_MAX];
Uint8 			enableRTG = 1;

static jo_camera    cam1;
static jo_camera    cam2;

static bool     	is_cd_playing = false;

static bool			show_debug = false;	
static bool			show_level_map = true;
Uint16				model_total = 0;

FIXED				max_gravity = toFIXED(20);//8
FIXED				wmax_gravity = toFIXED(10);//1

static bool			use_light = true;
static Sint16		section_dist;
static short     	crash_sound;
static short    	pup_sound;
static short     	cpoint_sound;
static short		boing_sound;
static short		explosion_sound;

static Uint16		enemy_total;
static Uint16		powerup_total;
int						preview_tex;
int						trackmap_tex;

Uint8				map_builder_delete_mode = false;
Uint8				map_builder_mode = 0;
Uint16				map_builder_model=0;
Uint16				map_builder_car=0;
Uint16				map_builder_powerup=0;
Uint16				gridsize = 32;
Uint16				total_sections = 0;
Uint16 				object_number = 0;
float				object_scale = 1;
Uint16				object_pol_num = 0;
Uint16				object_last_texture = 0;
Uint16				object_last_pol_num = 0;
bool				object_show_poly = 0;
Uint16				anim_tex_num;
Uint16				anim_frame_counter;
Uint16				gouraud_counter = 0;
Uint8				current_players = 0;
Uint8				cam_mode = 2;
Uint8				saved_cam_mode = 2;
Uint16				map_rx;
Sint16				rotate_cam;
Uint8				target_player = 0;
Uint8				total_waypoints = 0;
Uint8				sfx_volume = 4;
Uint8				cd_track = 0;
Uint8				music_vol = 6;
Sint16 				floor_x = 0;
Sint16 				floor_z = 0; 

//from XL2//
void ztClearText(void)
{
    Uint16 i;
    for (i=0; i<64; ++i)
        jo_nbg2_printf(0, i,"                                                                ");
}

int count_char(char * z)
{
	int m;
	int charcount;

	charcount = 0;
	for(m=0; z[m]; m++) {
		//if(z[m] != ' ') {
			charcount ++;
		//}
	}
	return charcount;
}

//XL2//
/**Simple function to draw the sprites with Color Lookup tables**/
void render_CLUT_sprite_HT(unsigned int id, int x, int y, int z)
{
    FIXED s_pos[XYZS];    s_pos[X]= x <<16; s_pos[Y]= y <<16; s_pos[Z]= z <<16; s_pos[S]=65536;
    SPR_ATTR spr_attributes = SPR_ATTRIBUTE(id, LUTidx(id), No_Gouraud, CL16Look | ECdis | HSSon | CL_Trans,  sprNoflip  );
    slPutSprite(s_pos , &spr_attributes , 0) ;
}

void render_CLUT_sprite(unsigned int id, int x, int y, int z)
{
    FIXED s_pos[XYZS];    s_pos[X]= x <<16; s_pos[Y]= y <<16; s_pos[Z]= z <<16; s_pos[S]= 65536;
    SPR_ATTR spr_attributes = SPR_ATTRIBUTE(id, LUTidx(id), No_Gouraud, CL16Look | ECdis | HSSon,  sprNoflip  );
    slPutSprite(s_pos , &spr_attributes , 0) ;
}

void render_CLUT_sprite_MESH(unsigned int id, int x, int y, int z)
{
    FIXED s_pos[XYZS];    s_pos[X]= x <<16; s_pos[Y]= y <<16; s_pos[Z]= z <<16; s_pos[S]= 65536;
    SPR_ATTR spr_attributes = SPR_ATTRIBUTE(id, LUTidx(id), No_Gouraud, CL16Look | ECdis | HSSon | MESHon,  sprNoflip  );
    slPutSprite(s_pos , &spr_attributes , 0) ;
}
//from top left of screen
void render_CLUT_sprite2(unsigned int id, int x, int y, int z)
{
    FIXED s_pos[XYZS];    
	s_pos[X]= jo_int2fixed(x - JO_TV_WIDTH_2 + JO_DIV_BY_2(__jo_sprite_def[id].width));
	s_pos[Y]= jo_int2fixed(y - JO_TV_HEIGHT_2 + JO_DIV_BY_2(__jo_sprite_def[id].height));
	s_pos[Z]=jo_int2fixed(z); 
	s_pos[S]=65536;
    SPR_ATTR spr_attributes = SPR_ATTRIBUTE(id, LUTidx(id), No_Gouraud, CL16Look | ECdis | HSSon,  sprNoflip  );
    slPutSprite(s_pos , &spr_attributes , 0) ;
}

void replace_texture(XPDATA * pol)
{
	Uint32 cnt, nbPt;
    nbPt = pol->nbPolygon;
	cnt = 0;
	
	for (cnt=0; cnt < nbPt; cnt++)
    {	
	pol->attbl[cnt].texno = anim_tex_num;
	pol->attbl[cnt].colno = LUTidx(anim_tex_num);
	}	
	
}


void		change_player_volume(int p, int vol, int pan)
{
pcm_parameter_change(players[p].eng1_sound, vol, pan);	
pcm_parameter_change(players[p].eng2_sound, vol, pan);
pcm_parameter_change(players[p].eng3_sound, vol, pan);
pcm_parameter_change(players[p].eng4_sound, vol, pan);
pcm_parameter_change(players[p].eng5_sound, vol, pan);
pcm_parameter_change(players[p].eng6_sound, vol, pan);
pcm_parameter_change(players[p].eng7_sound, vol, pan);
pcm_parameter_change(players[p].drift_sound, vol, pan);
pcm_parameter_change(players[p].horn_sound, vol, pan);
	
}

void		stop_engine_sound(int p)
{
	
pcm_cease(players[p].eng1_sound);
pcm_cease(players[p].eng2_sound);
pcm_cease(players[p].eng3_sound);
pcm_cease(players[p].eng4_sound);
pcm_cease(players[p].eng5_sound);
pcm_cease(players[p].eng6_sound);	
pcm_cease(players[p].eng7_sound);	
}

void 		stop_sounds(int p)
{
stop_engine_sound(p);
pcm_cease(players[p].drift_sound);
pcm_cease(players[p].horn_sound);
pcm_cease(pup_sound);
pcm_cease(crash_sound);
pcm_cease(cpoint_sound);
//pcm_cease(effect_sound);
pcm_cease(explosion_sound);
pcm_cease(boing_sound);
pcm_cease(players[p].siren_sound);

}

void init_2p_display(void)
{
	
	//jo_core_init(JO_COLOR_Black);
    slCurWindow(winNear);
	slWindow(JO_TV_WIDTH/2, 0, JO_TV_WIDTH-1, JO_TV_HEIGHT-1, DRAW_DISTANCE_MAX, JO_TV_WIDTH/4*3, JO_TV_HEIGHT_2); 
	slPerspective(FOV);	
	
	slCurWindow(winFar);
	slWindow(0, 0, JO_TV_WIDTH/2-1, JO_TV_HEIGHT-1, DRAW_DISTANCE_MAX, JO_TV_WIDTH/4, JO_TV_HEIGHT_2); 
	slPerspective(FOV);		

	//slScrWindowModeNbg1(win0_IN);
   // slScrWindowModeNbg0(win0_OUT);	
	jo_core_set_screens_order(JO_NBG2_SCREEN, JO_SPRITE_SCREEN, JO_RBG0_SCREEN);
	
		
}

void init_1p_display(void)
{
	
	//jo_core_init(JO_COLOR_Black);
    
  //  jo_3d_camera_init(&cam1);
	slCurWindow(winFar);
	slWindow(0, 0, JO_TV_WIDTH-1, JO_TV_HEIGHT-1, DRAW_DISTANCE_MAX, JO_TV_WIDTH_2, JO_TV_HEIGHT_2); //Includes the draw distance. Also I left 40 pixels for a HUD.
	slPerspective(FOV);	
	//jo_3d_camera_init(&cam2);
	slCurWindow(winNear);
	slWindow(0, 0, JO_TV_WIDTH-1, JO_TV_HEIGHT-1, DRAW_DISTANCE, JO_TV_WIDTH_2, JO_TV_HEIGHT_2); //Includes the draw distance. Also I left 40 pixels for a HUD.
	//slScrWindowModeNbg1(win0_IN);
    //slScrWindowModeNbg0(win0_IN);
		
	jo_core_set_screens_order(JO_NBG2_SCREEN, JO_SPRITE_SCREEN, JO_RBG0_SCREEN);
	
		
}

void effect(int p, int type, int effect_x, float effect_y, int effect_z)
{
	
	switch(type)
	{
	case 0: 	
				players[p].effect_type = 0;
				players[p].effect_size = 0.0f;
				players[p].effect_x = effect_x;
				players[p].effect_y = effect_y;
				players[p].effect_z = effect_z;
				break;	

	case 1: 	
				players[p].effect_type = 1;
				
				players[p].effect_size = 0.0f;
				players[p].effect_x = effect_x;
				players[p].effect_y = effect_y;
				players[p].effect_z = effect_z;
				break;	
				
	case 2: 	
				players[p].effect_type = 2;
				
				players[p].effect_size = 0.0f;
				players[p].effect_x = effect_x;
				players[p].effect_y = effect_y;
				players[p].effect_z = effect_z;
				break;	
				
	case 3: 	
				players[p].effect_type = 3;
				
				players[p].effect_size = 0.0f;
				players[p].effect_x = effect_x;
				players[p].effect_y = effect_y;
				players[p].effect_z = effect_z;
				break;	
		
	}
	
	
}


/* 8 tiles */
static const jo_tile    PUP_Tileset[] =
{
	{0, 0, 32, 32},
	{0, 32, 32, 32},
	{0, 64, 32, 32},
	{0, 96, 32, 32},
	{32, 0, 32, 32},
	{32, 32, 32, 32},
	{32, 64, 32, 32},
	{32, 96, 32, 32}
	
};
//44
static const jo_tile    MAP_Tileset[] =
{
	{0, 0, 48, 48},
	{0, 48, 48, 48},
	{0, 96, 48, 48},
	{0, 144, 48, 48},
	{0, 192, 48, 48},
	{48, 0, 48, 48},
	{48, 48, 48, 48},
	{48, 96, 48, 48},
	{48, 144, 48, 48},
	{48, 192, 48, 48},
	{96, 0, 48, 48},
	{96, 48, 48, 48},
	{96, 96, 48, 48},
	{96, 144, 48, 48},
	{96, 192, 48, 48},
	{144, 0, 48, 48},
	{144, 48, 48, 48},
	{144, 96, 48, 48},
	{144, 144, 48, 48},
	{144, 192, 48, 48},
	{192, 0, 48, 48},
	{192, 48, 48, 48},
	{192, 96, 48, 48},
	{192, 144, 48, 48},
	{192, 192, 48, 48},
	{240, 0, 48, 48},
	{240, 48, 48, 48},
	{240, 96, 48, 48},
	{240, 144, 48, 48},
	{240, 192, 48, 48},
	{288, 0, 48, 48},
	{288, 48, 48, 48},
	{288, 96, 48, 48},
	{288, 144, 48, 48},
	{288, 192, 48, 48},
	{336, 0, 32, 32},
	{336, 32, 32, 32},
	{336, 64, 32, 32},
	{336, 96, 32, 32},
	{336, 128, 32, 32},
	{336, 160, 32, 32},
	{336, 192, 32, 32},
	{336, 224, 16, 16},
	{352, 224, 16, 16}
	
};


void create_checkpoint(checkpoint* new_checkpoint, Uint16 section, Sint16 x, Sint16 y, Sint16 z, Sint16 ry)
{
	new_checkpoint->section = section;
	new_checkpoint->x = x;
	new_checkpoint->y = y;
	new_checkpoint->z = z;
	new_checkpoint->ry = ry;
	
}

void create_waypoint(waypoint* new_waypoint, Uint16 section, Sint16 x, Sint16 y, Sint16 z)
{
	new_waypoint->section = section;
	new_waypoint->x = x;
	new_waypoint->y = y;
	new_waypoint->z = z;
	
	total_waypoints ++;
		
}

int roundUp(Sint16 numToRound, Sint16 multiple)
{
    if (multiple == 0)
        return numToRound;

    int remainder = JO_ABS(numToRound) % multiple;
    if (remainder == 0)
        return numToRound;

    if (numToRound < 0)
        return -(JO_ABS(numToRound) - remainder);
    else
        return numToRound + multiple - remainder;
}

void animate_texture(Uint16 start_tex, Uint8 total_frames, Uint8 speed)
{
if (game.game_state != GAMESTATE_GAMEPLAY)
       return;
   if(anim_tex_num == 0)
   {
	   anim_tex_num = start_tex;
   }
   
   if(anim_frame_counter >= speed)
   {
	   anim_frame_counter = 0;
	   if(anim_tex_num == start_tex+total_frames)
	   {
			anim_tex_num = start_tex;
	   }else
	   {
	   anim_tex_num ++;
	   }
   }else
   {
	   anim_frame_counter ++;
   }
   
   
  
	
}

void apply_player_gravity(int p)
{
	if(players[p].in_water)
	{
		if (players[p].physics_speed_y < wmax_gravity)
		players[p].physics_speed_y += players[p].physics_wgravity;
	}else
	{
		if (players[p].physics_speed_y < max_gravity)
		players[p].physics_speed_y += players[p].physics_gravity;
	}
	
}

void getTime(jo_datetime* currentTime)
{
    SmpcDateTime *time = NULL;

    slGetStatus();

    time = &(Smpc_Status->rtc);

    currentTime->day = slDec2Hex(time->date);
    currentTime->year = slDec2Hex(time->year);
    currentTime->month = time->month & 0x0f;

    currentTime->hour = (char)slDec2Hex(time->hour);
    currentTime->minute = (char)slDec2Hex(time->minute);
    currentTime->second = (char)slDec2Hex(time->second);
}

unsigned int getSeconds()
{
    jo_datetime now = {0};
    unsigned int numSeconds = 0;

    getTime(&now);

    numSeconds = now.second + (now.minute * 60) + (now.hour * (60*60)) + (now.day * (24*60*60));

    return numSeconds;
}




void reset_demo(void)
{
game.game_state = GAMESTATE_RACE_START;
game.pause_start_time = 0;
if(game.players == 1)
{
game.enable_shadows = true;
}else
{
game.enable_shadows = false;
}

game.finishing_position = 0;


//current_players = game.players;

for(int p = 0; p < game.players; p++)
{
players[p].pause_time = 0;

players[p].cam_pos_x = CAM_DEFAULT_X;
players[p].cam_pos_y = CAM_DEFAULT_Y;
players[p].cam_height = CAM_DEFAULT_Y;
players[p].cam_pos_z = CAM_DEFAULT_Z;
players[p].cam_dist = CAM_DIST_DEFAULT;
players[p].cam_angle_x = 0;
players[p].cam_angle_y = 0;
players[p].cam_angle_z = 0;
players[p].cam_target_x = 0;
players[p].cam_target_y = 0;
players[p].cam_target_z = 0;
players[p].cam_zoom_num = 1;

players[p].x = 	game.start_x + players[p].start_x;
players[p].y = 	players[p].start_y;
players[p].z =	game.start_z + players[p].start_z;
players[p].ry = 0;
players[p].physics_is_in_air = false;

players[p].current_checkpoint = 0;
players[p].best_time = 0;
players[p].total_time = 0;
players[p].current_time = 0;
players[p].laps = 0;
players[p].start_time = 0;

players[p].can_be_hurt = true;
players[p].can_make_clouds = false;
players[p].shadow_y = players[p].y;
players[p].shadow_size = 1.0f;
players[p].effect_size = 2.0f;

players[p].health = 100;
players[p].current_powerup = 0;
apply_player_gravity(p);
players[p].projectile_alive = false;
players[p].explosion_size = 11;

players[p].physics_speed_y = 0;
players[p].physics_speed = 0;

players[p].cpu_left = 0;
players[p].cpu_right = 0;
players[p].cpu_gas = 0;
players[p].cpu_brake = 0;
players[p].cpu_siren = 0;

players[p].current_waypoint = 0;
players[p].next_waypoint = 0;

stop_sounds(p);
players[p].volume = sfx_volume;

players[p].object_scale = 1.0f;
players[p].enable_controls = false;
players[p].race_ended = false;

players[p].position = 0;
players[p].physics_speed_x_adj = 0;
players[p].physics_speed_z_adj = 0;

players[p].powerup_active = false;


}
game.race_end_timer = 0;

target_player = 0;

for(Uint16 e = 0; e < enemy_total; e++)
	{
		enemies[e].alive = true;
		enemies[e].x = enemies[e].start_x;
		enemies[e].y = enemies[e].start_y;
		enemies[e].z = enemies[e].start_z;
		enemies[e].jump_timer = ENEMY_JUMP_TIMER;
		enemies[e].health = enemies[e].start_health;
		enemies[e].rz = 0;
	}

}

void reset_to_last_checkpoint(int p)
{
	int previous_checkpoint;
	
	if(players[p].current_checkpoint > 0)
	{
	previous_checkpoint = players[p].current_checkpoint - 1;
	}else
	{
	previous_checkpoint = 0;
	}
	players[p].x = checkpoints[previous_checkpoint].x + map_section[checkpoints[previous_checkpoint].section].x;
	players[p].y = checkpoints[previous_checkpoint].y + map_section[checkpoints[previous_checkpoint].section].y;
	players[p].z = checkpoints[previous_checkpoint].z + map_section[checkpoints[previous_checkpoint].section].z;
	players[p].nextx = players[p].x;
	players[p].nexty = players[p].y;
	players[p].nextz = players[p].z;
	players[p].ry = checkpoints[previous_checkpoint].ry;
	
	players[p].physics_speed = 0;
	players[p].physics_speed_x_adj = 0;
	players[p].physics_speed_z_adj = 0;
	players[p].physics_speed_y = 0;
	
	stop_sounds(p);
	
	pcm_play(pup_sound,PCM_SEMI, 6);

}


void				player_jump(int p)
{			
			players[p].physics_speed_y = players[p].physics_jump_speed_y;
		
}

void				player_high_bounce(int p)
{
			stop_sounds(p);
			pcm_play(boing_sound,PCM_SEMI, 6);
			
			players[p].physics_speed_y = -24 <<	16;
		
}

void				player_bounce(int p)
{
			stop_sounds(p);
			pcm_play(boing_sound,PCM_SEMI, 6);
			
			players[p].physics_speed_y = -16 <<	16;
		
}

void				player_ramp_jump(int p)
{
	
			players[p].physics_speed_y = -toFIXED(players[p].physics_speed*2);	
		
}

void pup_timer_counter(int p)
{
	players[p].pup_timer += framerate;//1*framerate
	if(players[p].pup_timer >= PUP_TIMER)
	{
	players[p].powerup_active = false;
	players[p].current_powerup = 0;
	players[p].pup_timer = 0;
	powerups[players[p].powerup_id].used = false;
	effect(p, 1,powerups[players[p].powerup_id].x, powerups[players[p].powerup_id].y, powerups[players[p].powerup_id].z);
	players[p].object_scale = 1.0f;
	}
		
}

void player_hurt(int p)
{
	players[p].hurt_timer += framerate;//1*framerate
	players[p].ry+=90;
			
			if(players[p].hurt_timer >= PLAYER_HURT_TIMER)
			{
				players[p].can_be_hurt = true;
				players[p].hurt_timer = 0;
				
			}
			if(players[p].health <= 0)
			{
			reset_demo();
			}

}


void	set_player_position(int p)
{
	
	if(players[p].laps <= MAX_LAPS )
	{
		//distance to next waypoint
		players[p].dist_to_next_waypoint = JO_ABS(players[p].x - waypoints[players[p].next_waypoint].x) + JO_ABS(players[p].y - waypoints[players[p].next_waypoint].y) + JO_ABS(players[p].z - waypoints[players[p].next_waypoint].z);
				
		players[p].position = 1;
		for(int op = 0; op < game.players; op++)
			{
				
				if(op != p)
				{
					if(players[p].laps < players[op].laps)
					{
					players[p].position ++;
					}
					else if(players[p].laps == players[op].laps)
					{
						if(players[p].current_waypoint < players[op].current_waypoint)
						{
						players[p].position ++;	
						}
						else if(players[p].current_waypoint == players[op].current_waypoint)
						{
							if(players[p].dist_to_next_waypoint > players[op].dist_to_next_waypoint)//players[p].finish
							{
							players[p].position ++;
							}
						}
					}
					
				}
				
			}
			
	}

}	

void	set_player_total_position(int p)
{

		players[p].total_position = 1;
		for(int op = 0; op < game.players; op++)
			{				
				if(op != p)
				{
					if(players[p].total_points < players[op].total_points || (players[p].total_points == players[op].total_points && p < op))
					{
					players[p].total_position ++;
					}
						
				}
				
			}

}

void				transition_to_end_level(void)
{
	
	if (is_cd_playing)
        {
             CDDAStop();
            is_cd_playing = false;
        }
	
	target_player = game.winner;
	players[target_player].cam_zoom_num = 3;
	
	//save players camera setting and set up camera for end level screen
	saved_cam_mode = cam_mode;
	cam_mode = 0;
	
	for(int p = 0; p < game.players; p++)
	{
	stop_sounds(p);
	
	//apply points and add to total points
	players[p].points = points_table[players[p].position];
	players[p].total_points += players[p].points;
	set_player_position(p);
	
	game.finishing_position ++;

	
	if(game.mode == GAMEMODE_1PLAYERRACE && players[p].position == 0)
	{
		//apply finishing position to non finishing cpu players
		players[p].position = game.finishing_position+1;
		game.finishing_position ++;

	}	
	players[p].cam_pos_x = 0;
	players[p].cam_pos_y = 0;
	players[p].cam_height = 0;
	players[p].cam_pos_z = 0;
	players[p].cam_dist = 0;
	players[p].cam_angle_x = 0;
	players[p].cam_angle_y = 0;
	players[p].cam_angle_z = 0;
	players[p].cam_target_x = 0;
	players[p].cam_target_y = 0;
	players[p].cam_target_z = 0;
	
	}
	
	for(int p = 0; p < game.players; p++)
	{
	set_player_total_position(p);	
	}
	game.show_total_table = 0;
	game.game_state = GAMESTATE_END_LEVEL;
	ztClearText();
	//init_1p_display();
	
}

void race_ended(void)
{
if(game.race_end_timer < RACE_END_TIMER)
	{
	game.race_end_timer ++;	
	//stop players controls
	}else
	{
	transition_to_end_level();
	}

}

void player_collision_handling(int p)
{
	Sint16 collide;
	bool ycollide = 0;
	bool xcollide = 0;
	bool zcollide = 0;
	bool rcollide = 0;
	bool tcollide = 0;

	
	Uint16 x_dist;
	Uint16 y_dist;
	Uint16 z_dist;
	Uint16 collpoints_total;
	Sint16 collpoints_x;
	Sint16 collpoints_y;
	Sint16 collpoints_z;
	Uint16 collpoints_xsize;
	Uint16 collpoints_ysize;
	Uint16 collpoints_zsize;
	Sint8 collpoints_rot;
	Uint8 collpoints_type;
	Uint8 trigger;
	FIXED	SinR, CosR;
	
	if(!players[p].can_be_hurt)
		{
		player_hurt(p);
		}	
		
	players[p].nextx = players[p].x;
	players[p].nexty = players[p].y;
	players[p].nextz = players[p].z;
	
	//if(!players[p].physics_is_in_air)
	//{
	players[p].ary = players[p].ry;	
	//}
	
	SinR = jo_sin(players[p].ary);
	CosR = jo_cos(players[p].ary);

	players[p].physics_speed_x = (players[p].physics_speed * players[p].physics_grip);
	players[p].physics_speed_z =  players[p].physics_speed;
	
	players[p].delta_x = players[p].physics_speed_x_adj + ((players[p].physics_speed_x*CosR + players[p].physics_speed_z*SinR)/32768);
	players[p].delta_z = players[p].physics_speed_z_adj + ((players[p].physics_speed_z*CosR - players[p].physics_speed_x*SinR)/32768);
	
	players[p].nextx += players[p].delta_x;
	players[p].nexty += players[p].physics_speed_y>>16;
	players[p].nextz += players[p].delta_z;
	
	///collide with other player
	
		if(game.mode == GAMEMODE_1PLAYERSURVIVAL && players[0].health <=0)
		{
		transition_to_end_level();
		}
		
		if(game.players > 1)
		{
			
			for(int op = 0; op < game.players; op++)
			{
				if(op != p)
				{
			
						
			//x collide
			if(players[p].current_powerup == 7)
			{
			collide = has_object_collision(players[p].nextx, players[p].y, players[p].z, INV_SIZE, players[p].ysize, INV_SIZE,
								players[op].x, players[op].y, players[op].z, players[op].xsize, players[op].ysize, players[op].zsize);	
			}else
			{
			collide = has_object_collision(players[p].nextx, players[p].y, players[p].z, players[p].xsize, players[p].ysize, players[p].zsize,
								players[op].x, players[op].y, players[op].z, players[op].xsize, players[op].ysize, players[op].zsize);
			}
			
			if(collide)
			{	stop_sounds(p);
				pcm_play(crash_sound, PCM_PROTECTED, players[p].volume);
								
				players[op].physics_speed_x_adj = (players[p].delta_x - players[op].delta_x)*2;
				players[p].nextx = players[p].x;
				//players[p].physics_speed_x_adj = -players[p].delta_x;
			
								
				if(game.mode == GAMEMODE_1PLAYERSURVIVAL)
				{
				players[p].health -=10;	
				}
				
				if(players[p].current_powerup == 7 && players[op].can_be_hurt)
				{
				players[op].explosion_size = 0;
				players[op].px = players[op].x; players[op].py = players[op].y; players[op].pz = players[op].z;
				stop_sounds(op);
				pcm_play(explosion_sound, PCM_SEMI, 6);
				
					if(players[op].current_powerup != 5 && players[op].current_powerup != 7)
					{
						players[op].can_be_hurt = false;	
						player_jump(op);
						players[op].physics_speed = 0.0f;
						
						if(game.mode == GAMEMODE_1PLAYERSURVIVAL)
						{
						players[op].health -=20;	
						}
					
					}
				}
									
			}
			
			//z collide
			if(players[p].current_powerup == 7)
			{
			collide = has_object_collision(players[p].x, players[p].y, players[p].nextz, INV_SIZE, players[p].ysize, INV_SIZE,
								players[op].x, players[op].y, players[op].z, players[op].xsize, players[op].ysize, players[op].zsize);	
			}else
			{
			collide = has_object_collision(players[p].x, players[p].y, players[p].nextz, players[p].xsize, players[p].ysize, players[p].zsize,
								players[op].x, players[op].y, players[op].z, players[op].xsize, players[op].ysize, players[op].zsize);					
			}
			
			if(collide)
			{	
			stop_sounds(p);
			pcm_play(crash_sound, PCM_PROTECTED, players[p].volume);
			
			players[op].physics_speed_z_adj = (players[p].delta_z - players[op].delta_z)*2;
			players[p].nextz = players[p].z;
			//players[p].physics_speed_z_adj = -players[p].delta_z;
			
				if(game.mode == GAMEMODE_1PLAYERSURVIVAL)
				{
				players[p].health -=10;	
				}
				
				if(players[p].current_powerup == 7 && players[op].can_be_hurt)
				{
				players[op].explosion_size = 0;
				players[op].px = players[op].x; players[op].py = players[op].y; players[op].pz = players[op].z;
				stop_sounds(op);
				pcm_play(explosion_sound, PCM_SEMI, 6);
				
					if(players[op].current_powerup != 5 && players[op].current_powerup != 7)
					{
						players[op].can_be_hurt = false;	
						player_jump(op);
						players[op].physics_speed = 0.0f;
						
						if(game.mode == GAMEMODE_1PLAYERSURVIVAL)
						{
						players[op].health -=20;	
						}
					
					}
				}
									
			}
			
		
			///projectile collides with other player	
			if(players[p].projectile_alive)
			{
						
				collide = has_object_collision(players[p].px, players[p].py, players[p].pz, 50, 50, 50,
								players[op].x, players[op].y, players[op].z, players[op].xsize, players[op].ysize, players[op].zsize);	
			
				if(collide && players[op].can_be_hurt)
				{	
				players[p].explosion_size = 0;
				players[p].projectile_alive = false;
				stop_sounds(p);
				//jo_audio_play_sound_on_channel(&explosion_sound, 3);
				pcm_play(explosion_sound, PCM_SEMI, 6);
				
					if(players[op].current_powerup != 5)
					{
						players[op].can_be_hurt = false;	
						player_jump(op);
						players[op].physics_speed = 0.0f;
						
						if(game.mode == GAMEMODE_1PLAYERSURVIVAL)
						{
						players[op].health -=20;	
						}
					
					}
			
				}
			}
			
			}//player alive
				
			}//for loop
		
		}
	
	//map collision
	for(Uint16 i = 0; i < total_sections; i++)
	{
	//set map section distance	
	x_dist = JO_ABS(players[p].x - map_section[i].x);
	y_dist = JO_ABS(players[p].y - map_section[i].y);
	z_dist = JO_ABS(players[p].z - map_section[i].z);
	
	section_dist = x_dist + y_dist + z_dist;
	
		if(total_sections <5 || section_dist < COLL_DIST)
		{
			collpoints_total = map_section[i].a_cdata->nbCo;
			
			for(Uint16 c = 0; c < collpoints_total; c++)
			{
				collpoints_x = map_section[i].a_collison[c].cen_x + map_section[i].x + map_section[i].tx;
				collpoints_y = map_section[i].a_collison[c].cen_y + map_section[i].y + map_section[i].ty;
				collpoints_z = map_section[i].a_collison[c].cen_z + map_section[i].z + map_section[i].tz;
				
				collpoints_xsize = map_section[i].a_collison[c].x_size;
				collpoints_ysize = map_section[i].a_collison[c].y_size;
				collpoints_zsize = map_section[i].a_collison[c].z_size;
				collpoints_type = map_section[i].a_collison[c].att;
				
				collpoints_rot = map_section[i].a_collison[c].rot;
				
					
					
			
				///has trigger collision
				if(!tcollide && collpoints_type >=8)
				{
					trigger = has_trigger_collision(collpoints_type, collpoints_x, collpoints_y, collpoints_z, collpoints_xsize, collpoints_ysize, collpoints_zsize,players[p].x,players[p].y,players[p].z,players[p].xsize,players[p].ysize,players[p].zsize);
					if(trigger != NO_RAMP_COLLISION)
					{ 
						
						if(trigger == 8)//rotating platform x
						{
						players[p].nextz +=6;
						}
				
						if(trigger >= 10 && trigger <= 19)//checkpoint
						{
							int next_checkpoint;
			
							if(players[p].current_checkpoint == 4)
							{
							next_checkpoint = 1;
							}else
							{
							next_checkpoint = players[p].current_checkpoint + 1;
							}
							
						if(trigger - 9 == next_checkpoint || trigger - 9 == players[p].current_checkpoint)
						{
						
							if(trigger == 10 && (players[p].current_checkpoint == 0 || players[p].current_checkpoint == 4))
							{//START
								if(players[p].current_time < players[p].best_time || players[p].best_time == 0)
								{
								players[p].best_time = players[p].current_time;	
								}
							players[p].laps ++;
							
							if(game.mode == GAMEMODE_TIMEATTACK)
							{
								if(players[p].laps > 3 )// need to change to max laps and add multiplier to target time
								{
								players[p].total_time = players[p].current_time;	
								players[p].enable_controls = false;	
								players[p].race_ended = true;
								
								}
								
							}else if(game.mode == GAMEMODE_2PLAYERVS)
							{
							
								if(players[p].laps > MAX_LAPS )
								{
									players[p].wins ++;
									game.winner = p;
														
								//transition_to_end_level();
								players[p].enable_controls = false;	
								players[p].race_ended = true;
								}
								
								players[p].total_time += players[p].current_time;
								players[p].start_time = getSeconds();	
								players[p].pause_time = 0;
								game.pause_start_time = 0;
								
							}else if(game.mode == GAMEMODE_1PLAYERRACE)
							{
								
								if(players[p].laps > MAX_LAPS )
								{
									//disable controls if cpu player
									//if(p > 0)
									//{
									players[p].enable_controls = false;	
									players[p].race_ended = true;
									//}
									
									//record finishing position
									
									players[p].position = game.finishing_position+1;
									game.finishing_position ++;
									
									//record winner
									if(players[p].position == 1)
									{
										players[p].wins ++;
										game.winner = p;
										
										
									}
									
									//end race if player 1 finishes
									//if(p == 0)
									//{
									//game.finishing_position ++;
									//transition_to_end_level();
									
									//}
									
									

								
								}
								
								players[p].total_time += players[p].current_time;
								players[p].start_time = getSeconds();	
								players[p].pause_time = 0;
								game.pause_start_time = 0;
								
							}else 
							{
								
							players[p].total_time += players[p].current_time;
							players[p].start_time = getSeconds();	
							players[p].pause_time = 0;
								game.pause_start_time = 0;
							}
							
							
							stop_sounds(p);
							//jo_audio_play_sound_on_channel(&cpoint_sound, 0);
							pcm_play(cpoint_sound, PCM_PROTECTED, 6);
							} 
										
						players[p].current_checkpoint = trigger - 9;	
						
						//find players correct rotation based on next waypoint for respawn
						players[p].tx = waypoints[players[p].next_waypoint].x + map_section[waypoints[players[p].next_waypoint].section].x;
						players[p].ty = waypoints[players[p].next_waypoint].y + map_section[waypoints[players[p].next_waypoint].section].y;
						players[p].tz = waypoints[players[p].next_waypoint].z + map_section[waypoints[players[p].next_waypoint].section].z;
											
						checkpoints[trigger-10].ry = jo_atan2f((players[p].tx - players[p].x),(players[p].tz - players[p].z)) ;
						
						
						//checkpoints[trigger-10].ry = players[p].ry;
						}else 
						{
						reset_to_last_checkpoint(p);	
						}
						
						}
						
						// record waypoint
						if(trigger >= 100 && trigger <= 199)//waypoint
						{
						players[p].current_waypoint = trigger - 100;
						if(players[p].current_waypoint >= total_waypoints - 1)
						{
						players[p].next_waypoint = 0;	
						}else
						{
						players[p].next_waypoint = 	players[p].current_waypoint + 1;
						}
												
						}
						
						// bouncing platform
						if(trigger == 20 )//spring
						{
						player_high_bounce(p);
						}
						
						// ramp jump
						if(trigger == 21 )
						{
						player_ramp_jump(p);
						}
						
						// death zone
						if(trigger == 9 )
						{
						reset_to_last_checkpoint(p);
						}
						
						tcollide = true;
				
				
					}
				}
				
				//check if any of the next positions cause a collision before testing which one
				if(collpoints_type < 8)
				{
					collide = has_collision(collpoints_type, collpoints_x, collpoints_y, collpoints_z, collpoints_xsize, collpoints_ysize, collpoints_zsize,players[p].nextx,players[p].nexty,players[p].nextz,players[p].xsize,players[p].ysize,players[p].zsize);
					if(collide)
					{ 
															
						//VERTICAL COLLISION
						
						if(!ycollide && collpoints_type < 8)
						{
							collide = has_vertical_collision(collpoints_type, collpoints_x, collpoints_y, collpoints_z, collpoints_xsize, collpoints_ysize, collpoints_zsize,players[p].x,players[p].nexty,players[p].z,players[p].xsize,players[p].ysize,players[p].zsize);
							if(collide != NO_RAMP_COLLISION)
							{ 
								//players[p].int_height = collide;
								
								if(players[p].physics_speed_y <= 0)
								{players[p].can_jump = true;
							players[p].physics_is_in_air = false;
								}
								else
								{
								players[p].physics_speed_y = 0;
								
								//delta_y = 0.0f;
								players[p].nexty = collide;
								/*if(!rcollide)
								{
								players[p].nexty = players[p].int_height;
								}*/
										
								}
								
								players[p].current_map_section = i;
								players[p].current_collision = c;
							ycollide = true;//break;
							players[p].rx = 0;
							players[p].rz = 0;
							
							
							}
							else 
							{
								if (i == players[p].current_map_section && c == players[p].current_collision)
								{
									if (!players[p].on_ladder_x && !players[p].on_ladder_z )//&&!players[p].on_ceiling )
									{
									players[p].can_jump = false;
									players[p].physics_is_in_air = true;
									apply_player_gravity(p);
									
									players[p].rx = 0;	
									players[p].rz = 0;	
									}
								}
								
							}
						}
						
						///check for ramp collision
						if(!rcollide && collpoints_type < 8)
						{
							collide = has_ramp_collision(collpoints_type, collpoints_x, collpoints_y, collpoints_z, collpoints_xsize, collpoints_ysize, collpoints_zsize,players[p].nextx,players[p].nexty,players[p].nextz,players[p].xsize,players[p].ysize,players[p].zsize);
							if(collide != NO_RAMP_COLLISION)
							{
							rcollide = true;
							players[p].nexty = -collide;
							
							switch(collpoints_type)
							{
							case 1: players[p].rx = collpoints_rot;
									break;
									
							case 2: players[p].rz = collpoints_rot;
									break;
									
							case 3: players[p].rx = collpoints_rot;
									break;
									
							case 4: players[p].rz = collpoints_rot;
									break;	
							
							default: players[p].rx = 0;
									players[p].rz = 0;
									break;		
								
							}
							
							if(show_debug)
						{jo_nbg2_printf(0, 5, "RAMP: %3d",(int) collide);
						//jo_nbg2_printf(0, 6, "RDIS:\t%3d\t%3d\t%3d ",(int) x_dist, (int) y_dist, (int) z_dist);
						}
									
							}
						}
					
						if(game.enable_shadows && collpoints_type < 8)
						{
												
								///check for shadow collision
								collide = has_shadow_collision(collpoints_x, collpoints_y, collpoints_z, collpoints_xsize, collpoints_ysize, collpoints_zsize, players[p].x,players[p].y + players[p].ysize,players[p].z,players[p].xsize,players[p].ysize,players[p].zsize);
								if(collide != NO_RAMP_COLLISION && collpoints_type < 10)
								{ 
								players[p].shadow_y = players[p].y + collide;
								players[p].shadow_size = 1-((float)JO_ABS(players[p].y - players[p].shadow_y)/100);
								players[p].current_shadow_map_section = i;
								players[p].current_shadow_collision = c;
								
								}
								else
								{	
									if (i == players[p].current_shadow_map_section && c == players[p].current_shadow_collision)
									{
									players[p].shadow_size = -1;	
									}
								}
							
						}		
						///check for X axis collision
						if(!xcollide && collpoints_type == 0 )
						{
							collide = has_horizontal_collision(collpoints_type, collpoints_x, collpoints_y, collpoints_z, collpoints_xsize, collpoints_ysize, collpoints_zsize,players[p].nextx,players[p].y,players[p].z,players[p].xsize,players[p].ysize,players[p].zsize);
							if(collide != NO_RAMP_COLLISION)
							{			
								if(collide < 21 && players[p].physics_speed_y <= 0)
								{
								player_jump(p);
								}else
								{
								players[p].physics_speed_x_adj -=players[p].delta_x*1.5;
								players[p].physics_speed = 0.0f;//JJ physics change 28/01/2025
								players[p].nextx = players[p].x;
								pcm_play(crash_sound, PCM_PROTECTED, players[p].volume);
								xcollide = true;
								}			
								
							}
						}			
					
					
							///check for Z axis collision
						if(!zcollide && collpoints_type == 0)
						{
							collide = has_horizontal_collision(collpoints_type, collpoints_x, collpoints_y, collpoints_z, collpoints_xsize, collpoints_ysize, collpoints_zsize,players[p].x,players[p].y,players[p].nextz,players[p].xsize,players[p].ysize,players[p].zsize);
							if(collide != NO_RAMP_COLLISION)
							{ 
							
							if(collide < 21 && players[p].physics_speed_y <= 0)
								{
								player_jump(p);
								}else
								{
								players[p].physics_speed_z_adj -=players[p].delta_z*1.5;
								players[p].physics_speed = 0.0f;//JJ physics change 28/01/2025
								players[p].nextz = players[p].z;
								pcm_play(crash_sound, PCM_PROTECTED, players[p].volume);
								zcollide = true;
								}				
								
							}
						
						}		
					}// end 
				}
				///projectile collide with map
				if(players[p].projectile_alive)
				{
							
					if(collpoints_type ==0)
					{
						if(has_horizontal_collision(collpoints_type, collpoints_x, collpoints_y, collpoints_z, collpoints_xsize, collpoints_ysize, collpoints_zsize,players[p].px,players[p].py,players[p].pz,10,10,10))
						{
						switch(players[p].current_powerup)
							{
															
							case 2: //bomb
									players[p].speed_py = 0;
									break;
									
							case 3: //missile
									players[p].explosion_size = 0;
									players[p].projectile_alive = false;
									stop_sounds(p);
									pcm_play(explosion_sound, PCM_SEMI, 6);
									break;
									
														
							default: players[p].explosion_size = 0;
									players[p].projectile_alive = false;
									stop_sounds(p);
									pcm_play(explosion_sound, PCM_SEMI, 6);
									break;		
								
							}
						
						}
					}	
					
				}			
				
			
			}//end collpoint loop	
		
			
			
		
		}
	}//end map loop
	

	
	
	
	///has floor collision
	if(players[p].y >=250)
	{
	reset_to_last_checkpoint(p);
	}

	
		///has projectile has floor collision
	if(players[p].py >=132 && players[p].projectile_alive)
	{
	players[p].projectile_alive = false;	
	}
	
	
	
	
	
	///has powerup collision
	for(Uint16 w = 0; w < powerup_total; w++)
	{
		
		if(!powerups[w].used)
		{	
			collide = has_object_collision(players[p].x, players[p].y, players[p].z, players[p].xsize, players[p].ysize, players[p].zsize,
								powerups[w].x, powerups[w].y, powerups[w].z, 16, 16, 16);
			
			if(collide && players[p].current_powerup == 0 && players[p].enable_controls)
			{		
		
					if(powerups[w].type == 0)
					{
						if(game.mode == GAMEMODE_TIMEATTACK || players[p].position == game.players)
						{
						players[p].current_powerup = 1;
						}else
						{						
						players[p].current_powerup = jo_random(7);//random	
						}
						
						
					}else
					{
					players[p].current_powerup = powerups[w].type;	
					
					}
					//players[p].current_powerup = 2;//for testing
					players[p].pup_timer = 0;
					
					
					switch(players[p].current_powerup)
					{
							
					case 1:		
								//speed boost
								stop_sounds(p);
								pcm_play(pup_sound, PCM_SEMI, 6);
								powerups[w].used = true;
								players[p].powerup_id = w;
								effect(p,1,powerups[w].x, powerups[w].y, powerups[w].z);
								
								break;
							
					case 2:	
								//bomb
								stop_sounds(p);
								pcm_play(pup_sound, PCM_SEMI, 6);
								powerups[w].used = true;
								players[p].powerup_id = w;
								effect(p, 1,powerups[w].x, powerups[w].y, powerups[w].z);
								
								break;
							
					case 3:	
								//missile
								stop_sounds(p);
								pcm_play(pup_sound, PCM_SEMI, 6);
								powerups[w].used = true;
								players[p].powerup_id = w;
								effect(p, 1,powerups[w].x, powerups[w].y, powerups[w].z);
								
								break;
							
					case 4:	
								//make other players small
								
								stop_sounds(p);
								pcm_play(pup_sound, PCM_SEMI, 6);
								powerups[w].used = true;
								effect(p, 1,powerups[w].x, powerups[w].y, powerups[w].z);
								players[p].powerup_id = w;
								players[p].current_powerup = 0;
								
								for(int op = 0; op < game.players; op++)
								{
									if(op != p && players[op].current_powerup != 5)
									{
									players[op].current_powerup = 4;
									effect(op, 1,players[op].x, players[op].y, players[op].z);
									players[op].object_scale = 0.5f;
									players[op].powerup_active = true;
									}
								}
									
								break;
							
					case 5:	
								//shield
								players[p].powerup_active = true;
								stop_sounds(p);
								pcm_play(pup_sound, PCM_SEMI, 6);
								powerups[w].used = true;
								players[p].powerup_id = w;
								effect(p,1,powerups[w].x, powerups[w].y, powerups[w].z);
								
								break;
							
					case 6:	
								//spring
								stop_sounds(p);
								pcm_play(pup_sound, PCM_SEMI, 6);
								powerups[w].used = true;
								players[p].powerup_id = w;
								effect(p, 1,powerups[w].x, powerups[w].y, powerups[w].z);
								
								break;
								
					case 7:	
								//invincibility
								players[p].powerup_active = true;
								stop_sounds(p);
								pcm_play(pup_sound, PCM_SEMI, 6);
								powerups[w].used = true;
								players[p].powerup_id = w;
								effect(p, 1,powerups[w].x, powerups[w].y, powerups[w].z);
								
								break;
							
					
				
					}
								
			}
			
		}
		
	}
	
	
	
	
	
	
		///fire projectile
	if (players[p].shoot)
	{
		
			
	players[p].projectile_alive = true;	
	
	switch (players[p].current_powerup)
	{
	
	case 2: 	//bomb
		players[p].px = players[p].x;
		players[p].py = players[p].y;
		players[p].pz = players[p].z;
		
		//players[p].speed_px = (jo_cos(players[p].ry) + players[p].bomb_speed*jo_sin(players[p].ry))/32768;
		players[p].speed_py = -6;
		//players[p].speed_pz = (players[p].bomb_speed*jo_cos(players[p].ry) - jo_sin(players[p].ry))/32768;
		break;
	
	case 3:		//missile
		players[p].px = players[p].x;
		players[p].py = players[p].y - 10;
		players[p].pz = players[p].z;
		
		players[p].speed_px = (jo_cos(players[p].ry) + players[p].projectile_speed*jo_sin(players[p].ry))/32768;
		players[p].speed_py = 0;
		players[p].speed_pz = (players[p].projectile_speed*jo_cos(players[p].ry) - jo_sin(players[p].ry))/32768;
	
		break;
	
	default:break;
	
	}

		
	}
	///
	
	///has projectile timeout
	/*if((JO_ABS(players[p].x - players[p].px) + JO_ABS(players[p].y - players[p].py) + JO_ABS(players[p].z - players[p].pz)) > 1500);
	{
	players[p].projectile_alive = false;	
	}*/
	
	/*if(show_debug)
					{
					
					jo_nbg2_printf(0, 7, "NEXTX  %2d" ,players[p].nextx - players[p].x);
					
					
					}
					
	if(show_debug)
					{
					
					jo_nbg2_printf(0, 5, "LADDER  %d %d" ,(int) players[p].on_ladder_x, xcollide);
					jo_nbg2_printf(0, 6, "CEILING %d %d" ,(int) players[p].on_ceiling, ccollide);
					
					}*/
	//respawn stuck players
	if(game.mode == GAMEMODE_1PLAYERRACE && p != 0 && players[p].laps > 0 && players[p].laps < MAX_LAPS)
	{
			if(players[p].nextx == players[p].x && players[p].nexty == players[p].y && players[p].nextz == players[p].z)
			{
			//start timer	
			players[p].stuck_timer ++;
				
			}else
			{
			//cancel timer
			players[p].stuck_timer =0;
			}
			
			if(players[p].stuck_timer >= PLAYER_STUCK_TIMER)
			{
			players[p].stuck_timer = 0;
			reset_to_last_checkpoint(p);
			}
			
	}
	
	
	players[p].x = players[p].nextx;
	players[p].y = players[p].nexty;
	players[p].z = players[p].nextz;
			
	if(players[p].current_powerup != 0 && players[p].powerup_active == true)
	{
	pup_timer_counter(p);	
	}
	
	if(players[p].race_ended && p == 0)
	{
	
	race_ended();
	}
	
	
	if(!ycollide)
	{
	players[p].can_jump = false;
	players[p].physics_is_in_air = true;
	apply_player_gravity(p);
	
		if(players[p].rx > 0)
		{
		players[p].rx--;	
		}
		
		if(players[p].rx < 0)
		{
		players[p].rx++;	
		}
		
		if(players[p].rz < 0)
		{
		players[p].rz++;	
		}
		if(players[p].rz > 0)
		{
		players[p].rz--;	
		}
	
	}
	
	//camera
	
	
	/*if(players[p].y <=0)
	{
	players[p].cam_pos_y = players[p].y + players[p].cam_height;
	
	players[p].cam_target_y = players[p].y;
	}else
	{
		//players[p].cam_pos_y = players[p].cam_height;
	}
	
	*/	
		
}



void create_enemy(enemy* new_enemy, Uint8 type, Sint16 x,Sint16 y, Sint16 z, Sint16 xdist, Sint16 zdist, Sint16 max_speed, Sint16 health)
{
	new_enemy->alive = true;
	new_enemy->type = type;
	new_enemy->start_health = health;
	new_enemy->health = health;
	new_enemy->can_be_hurt = true;
	new_enemy->start_x = x;
	new_enemy->start_y = y;
	new_enemy->start_z = z;
    new_enemy->x = x;
	new_enemy->y = y;
	new_enemy->z = z;
	new_enemy->xdist = xdist;
	new_enemy->zdist = zdist;
	new_enemy->max_speed = max_speed * framerate;
	new_enemy->explosion_size = 0;
	new_enemy->px = x;
	new_enemy->py = y;
	new_enemy->pz = z;
	new_enemy->shoot_wait = 0;
	new_enemy->anim_speed = ANIM_SPEED * framerate;
	

	if(type == 1)//spider
	{new_enemy->xsize = 19;
	new_enemy->ysize = 21;
	new_enemy->zsize= 19;
	}
	
	if(type == 2)//bat
	{new_enemy->xsize = 19;
	new_enemy->ysize = 21;
	new_enemy->zsize= 19;
	}
	if(type == 3)//frog
	{new_enemy->xsize = 19;
	new_enemy->ysize = 21;
	new_enemy->zsize= 19;
	new_enemy->rleg_rz = -120;
	new_enemy->lleg_rz = 128;
	new_enemy->jump_timer = ENEMY_JUMP_TIMER;
	}
	
}

void create_powerup(powerup* new_powerup, Uint8 type, Sint16 x,Sint16 y, Sint16 z)
{
	new_powerup->used = false;
	new_powerup->type = type;
	new_powerup->x = x;
	new_powerup->y = y;
	new_powerup->z = z;
	//  Set Model
	new_powerup->pup_model=(XPDATA *)pup_data[type];
	
	
}


/*void                create_waterfall(void)
{
	jo_start_sprite_anim_loop(wfall_anim);
    
}*/

void create_player(void)
{
	
	int start_adj_x = 0;
	int start_adj_z = 0;
	int pad_number = 0;
	int g_adj = 0;
    for(int p = 0; p < game.players; p++)
	{
   
	//  Set alive
    players[p].alive = true;
	players[p].health = 100;
	
	// set size (hit box)
	players[p].xsize = 11;
	players[p].ysize = 11;
	players[p].zsize = 11;

	
	if(p == 2 || p == 4 || p == 6 )
	{
	start_adj_x = 0;	
	start_adj_z -= 96;
	}
	
	
	players[p].start_x = -48 + start_adj_x;
	players[p].start_y = WORLD_DEFAULT_Y;
	players[p].start_z = WORLD_DEFAULT_Z + start_adj_z;
	start_adj_x +=96;
	
	players[p].physics_gravity = toFIXED(0.5f * framerate);//0.25
	players[p].physics_wgravity = toFIXED(0.05f * framerate);
	players[p].physics_jump_speed_y = toFIXED(-6.0f);
	
	players[p].physics_air_acceleration_strength = 0.08f;
    players[p].physics_acceleration_strength = 0.46f;
    players[p].physics_friction = 0.1f;
	players[p].physics_friction2 = 0.8f;
    players[p].physics_deceleration_strength = 0.3f;
    players[p].physics_max_speed = 6.0f;    
    players[p].physics_speed = 0.0f;
    players[p].physics_speed_y = 0;
    players[p].physics_is_in_air = false;
	players[p].physics_turn_speed = 5.0f;
	players[p].bomb_speed = 5 * framerate;
	players[p].projectile_speed = 10 * framerate;
	players[p].physics_boost_acceleration_strength = 0.6f;
	players[p].physics_boost_max_speed = 9.0f;
	players[p].physics_small_max_speed = 4.0f;
	players[p].volume = sfx_volume;
	players[p].object_scale = 1.0f;
	players[p].colour1 = 61952;
	players[p].colour2 = 45084;
	players[p].gstart = 600 + g_adj;
		
	g_adj += 70;
	
		
	//set gamepad
	players[p].gamepad = pad_number;
	if(game.players > 2)
	{
	pad_number += 1;
	}else
	{
	pad_number += 15;
	}
	
	}
}

void create_map_section(level_section* section, Uint8 type, Sint16 x, Sint16 y, Sint16 z)
{
	// set type
	section->type = type;
	//  Set Location
    section->x = x;
    section->y = y;
    section->z = z;
	
			
	//  Set Model
	section->map_model=(XPDATA *)xpdata_[type];
	section->map_model_lp=(PDATA *)pdata_LP_[type];
	
	//	Set Collision Data
	section->a_cdata=(CDATA *)cdata_[type];
	section->a_collison	=(COLLISON *)section->a_cdata->cotbl;
		
				
}

/**Taken from RB demo**/
static FIXED light[XYZ];
static ANGLE light_ang[XYZ];
void computeLight()
{
    FIXED light_init[XYZ] = { 37837, 37837, 37837 };

    slPushUnitMatrix();
    {

        slRotX(light_ang[X]);
        slRotY(light_ang[Y]);
        slRotZ(light_ang[Z]);
        slCalcVector(light_init, light);
    }
    slPopMatrix();
	
    light_ang[X] = DEGtoANG(90.0);
    light_ang[Y] = DEGtoANG(90.0);
    light_ang[Z] = DEGtoANG(90.0);
   
}

void			create_particles(int p)
{
	
		if(players[p].particle_number >=4)
		{
		players[p].particle_number = 0;
		}else
		{
		players[p].particles[players[p].particle_number].x = players[p].px;
		players[p].particles[players[p].particle_number].y = players[p].py;
		players[p].particles[players[p].particle_number].z = players[p].pz;
		players[p].particles[players[p].particle_number].ry = players[p].ry;
		players[p].particles[players[p].particle_number].size = 0.0f;
		players[p].particle_number ++;
		}	
	
		players[p].particle_timer += framerate;//1*framerate
		
		if(players[p].particle_timer >= PLAYER_CLOUD_TIMER)
		{
		players[p].can_make_particles = false;
		players[p].particle_timer = 0;
		}
	
	
}

void			create_clouds(int p)
{
	
		if(players[p].cloud_number >=4)
		{
		players[p].cloud_number = 0;
		}else
		{
		players[p].clouds[players[p].cloud_number].x = players[p].x;
		players[p].clouds[players[p].cloud_number].y = players[p].y;
		players[p].clouds[players[p].cloud_number].z = players[p].z;
		players[p].clouds[players[p].cloud_number].ry = players[p].ry;
		players[p].clouds[players[p].cloud_number].size = 0.0f;
		players[p].cloud_number ++;
		}	
	
		players[p].cloud_timer += framerate;//1*framerate
		
		if(players[p].cloud_timer >= PLAYER_CLOUD_TIMER)
		{
		players[p].can_make_clouds = false;
		players[p].cloud_timer = 0;
		}
	
	
}

void				animate_player(int p)
{

	if(players[p].alive)
	{

		if(players[p].physics_speed != 0)
		{
		players[p].wheel_rx += players[p].physics_speed;	
			
		}



		if (players[p].wheel_rx > 180)
				players[p].wheel_rx -=360;
			else if (players[p].wheel_rx <= -180)
				players[p].wheel_rx +=360;
	
	
	if(players[p].drift)
	{	
	
	players[p].can_make_clouds = true;	
	
	}else
	{
		pcm_cease(players[p].drift_sound);
	}
	
	if(players[p].can_make_clouds)
	{
	create_clouds(p);
	}	
	
	if(players[p].powerup_active)
	{
		if(players[p].current_powerup == 1)
		{
		players[p].px = players[p].x; players[p].py = players[p].y; players[p].pz = players[p].z;
		players[p].can_make_particles = true;	
		create_particles(p);	
		}else if(players[p].current_powerup == 3 && players[p].projectile_alive)
		{
		players[p].can_make_particles = true;	
		create_particles(p);	
		}
	}
	
	}
}



void				draw_powerups(powerup* current_powerup)
{
	
	
	if(!current_powerup->used)
	{
		slPushMatrix();
		{
			slTranslate(toFIXED(current_powerup->x), toFIXED(current_powerup->y), toFIXED(current_powerup->z));
			slRotX(DEGtoANG(0)); slRotY(DEGtoANG(current_powerup->ry)); slRotZ(DEGtoANG(0));
			{
			slPutPolygonX(current_powerup->pup_model, light);
			}
		}
		slPopMatrix();
		
		
	}
	
	if(current_powerup->ry >=360)
	{
	current_powerup->ry = 0;
	}else
	{
	current_powerup->ry++;	
	}
}

void				draw_player(int p)
{
	
	if(players[p].alive)
	{
	
	slSetGouraudColor(JO_COLOR_RGB(players[p].r, players[p].g, players[p].b));
	
	
	if(!players[p].can_be_hurt)
	{
	players[p].r = 255; players[p].g = 0; players[p].b = 0;
	}else if(players[p].effect_size <= 2.0f)
	{
	players[p].r = 228; players[p].g = 210; players[p].b = 242;	
	}else if(players[p].in_water)
	{
	players[p].r = 71; players[p].g = 245; players[p].b = 249;
	}else if(players[p].current_powerup == 7)
	{
	players[p].r = 255; players[p].g = 255; players[p].b = 0;
	}else
	{
		players[p].r = 255; players[p].g = 255; players[p].b = 255;
	}
	
	switch(players[p].type)
		{
        
				
		case 0:	
	
				
					
					
					
					
				slPushMatrix();
				{
					
					slTranslate(players[p].x << 16, (players[p].y << 16) + 367001, players[p].z << 16);
					slRotX(DEGtoANG(players[p].rx)); slRotZ(DEGtoANG(players[p].rz)); slRotY(DEGtoANG(players[p].ry+180)); 	
				
					jo_3d_set_scalef(players[p].object_scale,players[p].object_scale,players[p].object_scale);
					slPutPolygonX((XPDATA *)car_data[p], light);
					
					//wheel front_right
					slPushMatrix();
					{
					slTranslate(-642252, 124518, -871628);
					slRotY(DEGtoANG(players[p].wheel_ry));slRotX(DEGtoANG(players[p].wheel_rx)); 
										
					slPutPolygonX((XPDATA *)&xpdata_WHEEL_FR, light);
					}
					slPopMatrix();
					
					//wheel front_left
					slPushMatrix();
					{
					slTranslate(642252, 124518, -871628);
					slRotY(DEGtoANG(180));
					slRotY(DEGtoANG(players[p].wheel_ry));slRotX(DEGtoANG(-players[p].wheel_rx));
															
					slPutPolygonX((XPDATA *)&xpdata_WHEEL_FL, light);
					}
					slPopMatrix();
					
					//wheel rear_right
					slPushMatrix();
					{
					slTranslate(-642252, 124518, 570163);
					
					slRotX(DEGtoANG(players[p].wheel_rx));			
					slPutPolygonX((XPDATA *)&xpdata_WHEEL_RR, light);
					}
					slPopMatrix();
					
					//wheel rear_left
					slPushMatrix();
					{
					slTranslate(642252, 124518, 570163);
					slRotY(DEGtoANG(180));
					
					slRotX(DEGtoANG(-players[p].wheel_rx));				
					slPutPolygonX((XPDATA *)&xpdata_WHEEL_RL, light);
					}
					slPopMatrix();
					
					if(players[p].current_powerup == 2)
					{
						slPushMatrix();
						{
												
						slPutPolygonX((XPDATA *)&xpdata_ATT_GUN, light);
						}
						slPopMatrix();
					}
					if(players[p].current_powerup == 3)
					{
						slPushMatrix();
						{
												
						slPutPolygonX((XPDATA *)&xpdata_ATT_GUN, light);
						}
						slPopMatrix();
					}
					if(players[p].current_powerup == 1)
					{
						slPushMatrix();
						{
						
						
						slPutPolygonX((XPDATA *)&xpdata_ATT_ROCKET, light);
						}
						slPopMatrix();
						
						slPushMatrix();
						{
						
							if(players[p].powerup_active == true)
							{
							slPutPolygonX((XPDATA *)&xpdata_ATT_ROCKET_FIRE, light);
							}
						}
						slPopMatrix();
					}
					
					
				}
				slPopMatrix();
				
				
				
				if(players[p].shadow_size >0 && game.enable_shadows)
				{
				slPushMatrix();
				{
					
					slTranslate(players[p].x <<16, players[p].shadow_y <<16, players[p].z <<16);
					//slRotX(DEGtoANG(players[p].rx)); slRotY(DEGtoANG(players[p].ry)); slRotZ(DEGtoANG(players[p].rz));
					jo_3d_set_scalef(players[p].shadow_size,players[p].shadow_size,players[p].shadow_size);				
					slPutPolygon((PDATA *)&xpdata_ham_shadow);
				}
				slPopMatrix();
				}
				//drifting clouds
				for(int c = 0; c < 5; c++)
				{
					if(players[p].clouds[c].size < 10)
					{
						//jo_sprite_enable_half_transparency();
						slPushMatrix();
						{
						
						slTranslate(players[p].clouds[c].x <<16, (players[p].clouds[c].y <<16)+ 367001, players[p].clouds[c].z <<16);
						slRotY(DEGtoANG(players[p].clouds[c].ry+180));
						slTranslate(-642252, 124518, 570163);
						
						render_CLUT_sprite_MESH(3,0,0,0);
						}
						slPopMatrix();
						
						slPushMatrix();
						{
						
						slTranslate(players[p].clouds[c].x <<16, (players[p].clouds[c].y <<16) + 367001, players[p].clouds[c].z <<16);
						slRotY(DEGtoANG(players[p].clouds[c].ry+180));	
						slTranslate(642252, 124518, 570163);						
						render_CLUT_sprite_MESH(3,0,0,0);
						}
						slPopMatrix();
						//jo_sprite_disable_half_transparency();
						
					
					players[p].clouds[c].size ++;
					players[p].clouds[c].y -= 1;
					}
				}

				//particles
				for(int t = 0; t < 5; t++)
				{
					if(players[p].particles[t].size < 10)
					{
						//jo_sprite_enable_half_transparency();
						slPushMatrix();
						{
						
						slTranslate(players[p].particles[t].x <<16, (players[p].particles[t].y <<16)- 367001, players[p].particles[t].z <<16);
						slRotY(DEGtoANG(players[p].particles[t].ry+180));
						slTranslate(0, 0, 570163);
						
						render_CLUT_sprite_MESH(11,0,0,0);
						}
						slPopMatrix();
						
						
						
					
					players[p].particles[t].size ++;
					players[p].particles[t].y -= 1;
					}
				}				
				
				
				break;				

		}
		
		
			if(players[p].effect_size <= 2.0f)
			{
				switch(players[p].effect_type)
				{
					
					case 0:
						
						break;
						
					case 1:
						slPushMatrix();
						{
							
							slTranslate(toFIXED(players[p].x), toFIXED(players[p].y), toFIXED(players[p].z));
							//slRotY(DEGtoANG(players[p].ry));
							jo_3d_set_scalef(players[p].effect_size,players[p].effect_size,players[p].effect_size);			
							slPutPolygonX((XPDATA *)&xpdata_spin_effect,light);
						}
						slPopMatrix();
						break;
						
					case 2:
						slPushMatrix();
						{
							
							slTranslate(toFIXED(players[p].effect_x), toFIXED(players[p].effect_y), toFIXED(players[p].effect_z));
							jo_3d_set_scalef(players[p].effect_size,players[p].effect_size,players[p].effect_size);			
							slPutPolygonX((XPDATA *)&xpdata_splash_effect,light);
						}
						slPopMatrix();
						break;
				
				}
			
			players[p].effect_size +=0.2f;	
			
			}
	
	
	slSetGouraudColor(CD_White);
	}
	
	if(players[p].current_powerup == 7 && players[p].powerup_active == true)//invincible
	{
		slPushMatrix();
		{
							
		slTranslate(toFIXED(players[p].x), toFIXED(players[p].y), toFIXED(players[p].z));
		slRotY(DEGtoANG(players[p].effect_ry));
		jo_3d_set_scalef(2.0f,2.0f,2.0f);			
		slPutPolygonX((XPDATA *)&xpdata_invincible_effect,light);
		}
		slPopMatrix();
		players[p].effect_ry +=10;
		
			if (players[p].effect_ry > 180)
		players[p].effect_ry -=360;
	else if (players[p].effect_ry <= -180)
		players[p].effect_ry +=360;
	}
	
	if(players[p].current_powerup == 5 && players[p].powerup_active == true)//shield
	{
		jo_sprite_enable_half_transparency();
		jo_3d_draw_scaled_billboard(PUP_TILESET + 7,players[p].x, players[p].y, players[p].z,2.0f);
		jo_sprite_disable_half_transparency();
		/*slPushMatrix();
		{
		slTranslate(toFIXED(players[p].x), toFIXED(players[p].y), toFIXED(players[p].z));
		render_CLUT_sprite(2,0,0,0);
		}
		slPopMatrix();
		*/
	}
	
	
	if(players[p].projectile_alive)
			{
				if(players[p].current_powerup == 2)
				{
				//bomb
				//jo_3d_draw_scaled_billboard(9,players[p].px, players[p].py, players[p].pz,1.0f);
				slPushMatrix();
						{
						slTranslate(toFIXED(players[p].px), toFIXED(players[p].py), toFIXED(players[p].pz));
						render_CLUT_sprite(9,0,0,0);
						}
						slPopMatrix();
				}else
				{
					slPushMatrix();
					{
							
					slTranslate(toFIXED(players[p].px), toFIXED(players[p].py), toFIXED(players[p].pz));
					slRotY(DEGtoANG(players[p].ry));
					//jo_3d_set_scalef(players[p].effect_size,players[p].effect_size,players[p].effect_size);			
					slPutPolygonX((XPDATA *)&xpdata_MISSILE,light);
					}
					slPopMatrix();
					
				}
			players[p].px += players[p].speed_px;
			players[p].py += players[p].speed_py;
			players[p].pz += players[p].speed_pz;
			/*
			if (((players[p].speed_px-1) >= 0) ^ (players[p].speed_px >= 0))
			{
			//changed sign
			players[p].speed_px = 0;
			}else
			{
			players[p].speed_px --;
			}
			
			if (((players[p].speed_pz-1) >= 0) ^ (players[p].speed_pz >= 0))
			{
			//changed sign
			players[p].speed_pz = 0;
			}else
			{
			players[p].speed_pz --;
			}
			*/
			
			//players[p].speed_px --;
			if(players[p].current_powerup == 2)
			{
			players[p].speed_py ++;
			}
			//players[p].speed_pz --;
					
			}
	
	if(players[p].explosion_size <= 10)
	{
		slPushMatrix();
		{
							
		slTranslate(toFIXED(players[p].px), toFIXED(players[p].py), toFIXED(players[p].pz));
		jo_3d_set_scalef(players[p].explosion_size,players[p].explosion_size,players[p].explosion_size);			
		slPutPolygonX((XPDATA *)&xpdata_EXPLOSION,light);
		}
		slPopMatrix();
	
			
	players[p].explosion_size +=2;	
	}

	
	
}



void				map_builder_draw_section(void)
{
	
	
	if(map_builder_mode == 0)
	{
	
			slPushMatrix();
			{
				slTranslate(toFIXED(players[0].x), toFIXED(players[0].y), toFIXED(players[0].z));
				slRotX(DEGtoANG(players[0].rx)); slRotY(DEGtoANG(players[0].ry)); slRotZ(DEGtoANG(players[0].rz));
				jo_3d_set_scalef(object_scale,object_scale,object_scale);
				{	
					slPutPolygonX(xpdata_[map_builder_model], light);
					//slPutPolygon(pdata_LP_[map_builder_model]);
				}
			}
			slPopMatrix();
			
	}else if(map_builder_mode == 1)
	{
		slPushMatrix();
			{
				slTranslate(toFIXED(players[0].x), toFIXED(players[0].y), toFIXED(players[0].z));
				slRotX(DEGtoANG(players[0].rx)); slRotY(DEGtoANG(players[0].ry)); slRotZ(DEGtoANG(players[0].rz));
				jo_3d_set_scalef(object_scale,object_scale,object_scale);
				{	
					slPutPolygon(pdata_LP_[map_builder_model]);
				}
			}
			slPopMatrix();
		
	}else
	{
	 //  Draw collision Cubes
	int collpoints_x;
	int collpoints_y;
	int collpoints_z;
				
	int collpoints_xsize;
	int collpoints_ysize;
	int collpoints_zsize;
	int collpoints_type;
	
	int collcube = map_section[map_builder_model].a_cdata->nbCo;
	
		for(int c = 0; c < collcube; c++)
		{
			collpoints_x = map_section[map_builder_model].a_collison[c].cen_x;
			collpoints_y = map_section[map_builder_model].a_collison[c].cen_y;
			collpoints_z = map_section[map_builder_model].a_collison[c].cen_z;
						
			collpoints_xsize = map_section[map_builder_model].a_collison[c].x_size*2;
			collpoints_ysize = map_section[map_builder_model].a_collison[c].y_size*2;
			collpoints_zsize = map_section[map_builder_model].a_collison[c].z_size*2;
			
			collpoints_type = map_section[map_builder_model].a_collison[c].att;
								
			slPushMatrix();
			{
				slTranslate(toFIXED(players[0].x), toFIXED(players[0].y), toFIXED(players[0].z));
				slRotX(DEGtoANG(players[0].rx)); slRotY(DEGtoANG(players[0].ry)); slRotZ(DEGtoANG(players[0].rz));
				jo_3d_set_scalef(object_scale,object_scale,object_scale);
				{	
					slPushMatrix();
					{
						slTranslate(toFIXED(collpoints_x), toFIXED(collpoints_y), toFIXED(collpoints_z));
						jo_3d_set_scalef(collpoints_xsize,collpoints_ysize,collpoints_zsize);
						{	
							if(collpoints_type >=100)
							{
							slPutPolygon((PDATA *)&pdata_WP);
							}else
							{
							slPutPolygon((PDATA *)&pdata_CC);	
							}
						}
					}
					slPopMatrix();
				
				}
			}
			slPopMatrix();
		
		}
	
		
		
	}
	
}



void				map_builder_draw_powerup(void)
{
		slPushMatrix();
			{
				slTranslate(toFIXED(players[0].x), toFIXED(players[0].y), toFIXED(players[0].z));
				slRotX(DEGtoANG(players[0].rx)); slRotY(DEGtoANG(players[0].ry)); slRotZ(DEGtoANG(players[0].rz));
				jo_3d_set_scalef(object_scale,object_scale,object_scale);
				{
					slPutPolygonX((XPDATA *)pup_data[map_builder_powerup], light);
				}
			}
			slPopMatrix();
		
		
		
	
}

void				draw_walls(void)
{
					slPushMatrix();
					{						
						
					slPutPolygonX((XPDATA *)xpdata_WALLS, light);
						
					}
					slPopMatrix();
	
}

void				draw_map(level_section* section, Uint8 p)
{
	int x_dist;
	int y_dist;
	int z_dist;
	
	/*if(players[p].in_water)
	{
	slSetGouraudColor(JO_COLOR_RGB(71,245,249));
	}else
	{
		slSetGouraudColor(CD_White);
	}*/
	
	//set draw distance
	x_dist = JO_ABS(players[p].x - section->x);
	y_dist = JO_ABS(players[p].y - section->y);
	z_dist = JO_ABS(players[p].z - section->z);
	
		if(total_sections > 8)
		{
		
			if((x_dist + y_dist + z_dist) < DRAW_DISTANCE )
			{	
		
			slPushMatrix();
					{
						slTranslate(section->x <<16, section->y <<16, section->z <<16);
						
						{
							slPutPolygonX(section->map_model, light);
							
						}
					}
					slPopMatrix();
					
										
			}else if(((x_dist + y_dist + z_dist) < DRAW_DISTANCE_2) && section->map_model_lp->nbPolygon != 0 )
			{
					slPushMatrix();
					{
						slTranslate(section->x <<16, section->y <<16, section->z <<16);
						{
							slPutPolygon(section->map_model_lp);
						}
					}
					slPopMatrix();	
					
			}
		
		}else
		{
			slPushMatrix();
					{
						slTranslate(section->x <<16, section->y <<16, section->z <<16);
						if(game.level == 5 && section->type >0)
						{
						slRotX(DEGtoANG(map_rx));
						map_rx -=6;
						}
						{
							slPutPolygonX(section->map_model, light);
						}
					}
					slPopMatrix();
			
		}
		
		if(map_rx <=-360)
		{map_rx = 0;}
			
}


/* 23 tiles */
static const jo_tile    HUD_Tileset[] =
{
	{0, 0, 72, 16},
	{0, 16, 72, 16},
	{72, 0, 72, 16},
	{72, 16, 72, 16},
	{0, 32, 32, 16},
	{32, 32, 16, 16},
	{48, 32, 16, 16},
	{64, 32, 16, 16},
	{80, 32, 16, 16},
	{96, 32, 16, 16},
	{112, 32, 16, 16},
	{128, 32, 16, 16},
	{32, 48, 16, 16},
	{48, 48, 16, 16},
	{64, 48, 16, 16},
	{0, 48, 32, 16},
	{80, 48, 8, 8},
	{80, 56, 8, 8},
	{88, 48, 8, 8},
	{88, 56, 8, 8},
	{96, 48, 8, 8},
	{96, 56, 8, 8},
	{104, 48, 8, 8},
	
};

void draw_hud(void)
{
	if(game.game_state == GAMESTATE_END_LEVEL)
	return;

	Sint16 p1_laps = 0;
	Sint16 p2_laps = 0;
	Sint16 hbar_x_adj = 0;	 
	Sint16 hbar_x = (hbar_x_adj + (8 * (players[0].health/10))/2)-16;
	Sint16 map_adj;
	Sint16 map_x;
	Sint16 map_y;
	
				
		for(int p = 0; p < game.players; p++)
	{
		if(players[p].laps > 0)
		{
		if(players[p].start_time == 0)
		{
		players[p].start_time =	getSeconds();
		}
		if(game.game_state != GAMESTATE_PAUSED && !players[p].race_ended)
		{
		players[p].current_time = getSeconds() - players[p].start_time - players[p].pause_time;
		}
		
		}else
		{
		players[p].current_time = 0;	
		}			
		players[p].mins = players[p].current_time / 60;
		players[p].secs = players[p].current_time % 60;

	}

		if(!show_debug)
		{
			
		if(game.players == 2)
		{
			map_adj = -80;
			
			if(cam_mode == 0)
			{
			jo_sprite_draw3D(game.hud_sprite_id+1,-118, -94, 100);
			jo_sprite_draw3D(game.hud_sprite_id,-118, -110, 100);
			//draw current powerup sprite
			if(players[0].current_powerup != 0)
			{
			jo_sprite_draw3D(PUP_TILESET + players[0].current_powerup -1,-78, -103, 50);
			}
			jo_sprite_draw3D(game.hud_sprite_id+3,118, -94, 100);
			jo_sprite_draw3D(game.hud_sprite_id+2,118, -110, 100);
			//draw current powerup sprite
			if(players[1].current_powerup != 0)
			{
			jo_sprite_draw3D(PUP_TILESET + players[1].current_powerup -1,78, -103, 50);
			}			
			jo_nbg2_printf(1, 1,  "PLAYER1");
			jo_nbg2_printf(1, 3,  "%02d.%02d", players[0].mins,players[0].secs);
			
			jo_nbg2_printf(32, 1,  "PLAYER2");
			jo_nbg2_printf(32, 3,  "  %02d.%02d", players[1].mins,players[1].secs);
			}else
			{
			
			jo_sprite_draw3D(game.hud_sprite_id+1,-200, -94, 100);
			jo_sprite_draw3D(game.hud_sprite_id,-200, -110, 100);
			//draw current powerup sprite
			if(players[0].current_powerup != 0)
			{
			jo_sprite_draw3D(PUP_TILESET + players[0].current_powerup -1,-160, -103, 50);
			}
			
			//laps
			//jo_sprite_enable_half_transparency();
			jo_sprite_draw3D(game.hud_sprite_id + 4,-218, 103, 100);
			jo_sprite_change_sprite_scale(2);
			
			//jo_sprite_draw3D(game.hud_sprite_id + 5 + players[0].laps,-180, 103, 100);
			if(players[0].laps>9)
			{
			
			//divide laps by 10 to get first digit
			p1_laps = players[0].laps/10;
			jo_sprite_draw3D(game.hud_sprite_id + 5 + p1_laps,-180, 103, 100);
			//get modulus 10 to get second digit and move original sprite to right by 16
			p1_laps = players[0].laps % 10;		
			jo_sprite_draw3D(game.hud_sprite_id + 5 + p1_laps,-148, 103, 100);
			
			}else
			{
			p1_laps = players[0].laps;	
			jo_sprite_draw3D(game.hud_sprite_id + 5 + p1_laps,-180, 103, 100);
			}	
			
			jo_sprite_restore_sprite_scale();
			jo_sprite_disable_half_transparency();	
			
			jo_sprite_draw3D(game.hud_sprite_id+3,39, -94, 100);
			jo_sprite_draw3D(game.hud_sprite_id+2,39, -110, 100);
			//draw current powerup sprite
			//draw current powerup sprite
			if(players[1].current_powerup != 0)
			{
			jo_sprite_draw3D(PUP_TILESET + players[1].current_powerup -1,0, -103, 50);
			}
			//laps
			//jo_sprite_enable_half_transparency();
			jo_sprite_draw3D(game.hud_sprite_id + 4,4, 103, 100);
			jo_sprite_change_sprite_scale(2);
			
			//jo_sprite_draw3D(game.hud_sprite_id + 5 + players[1].laps,60, 103, 100);
			if(players[1].laps>9)
			{
			
			//divide laps by 10 to get first digit
			p2_laps = players[1].laps/10;
			jo_sprite_draw3D(game.hud_sprite_id + 5 + p2_laps,28, 103, 100);
			//get modulus 10 to get second digit and move original sprite to right by 16
			p2_laps = players[1].laps % 10;		
			jo_sprite_draw3D(game.hud_sprite_id + 5 + p2_laps,60, 103, 100);
			
			}else
			{
			p2_laps = players[1].laps;	
			jo_sprite_draw3D(game.hud_sprite_id + 5 + p2_laps,60, 103, 100);
			}	
			
			jo_sprite_restore_sprite_scale();
			//jo_sprite_disable_half_transparency();	
		
			jo_nbg2_printf(1, 1,  "PLAYER1");
			jo_nbg2_printf(1, 3,  "%02d.%02d", players[0].mins,players[0].secs);
			jo_nbg2_printf(1, 5,  "WINS  %2d", players[0].wins);
			
			jo_nbg2_printf(32, 1,  "PLAYER2");
			jo_nbg2_printf(32, 3,  "  %02d.%02d", players[1].mins,players[1].secs);
			jo_nbg2_printf(32, 5,  "WINS  %2d", players[1].wins);
				
			}
		}else
		{
		
		map_adj = -118;
		
		
		//left side
		jo_sprite_draw3D(game.hud_sprite_id+1,-118, -94, 100);
		jo_sprite_draw3D(game.hud_sprite_id,-118, -110, 100);
		
		
		//draw current powerup sprite
			if(players[target_player].current_powerup != 0)
			{
			jo_sprite_draw3D(PUP_TILESET + players[target_player].current_powerup -1,-78, -103, 50);
			}
			jo_sprite_draw3D(game.hud_sprite_id+3,118, -94, 100);
			jo_sprite_draw3D(game.hud_sprite_id+2,118, -110, 100);
		
		//right side
		
		jo_sprite_draw3D(game.hud_sprite_id+3,118, -94, 100);
		jo_sprite_draw3D(game.hud_sprite_id+2,118, -110, 100);
		
		if(game.mode == GAMEMODE_1PLAYERSURVIVAL)
		{
		jo_nbg2_printf(32, 1,  "HEALTH");
		//health bar
		
		jo_sprite_change_sprite_scale_xy(players[target_player].health/20,1);
		
		jo_sprite_draw3D(game.hud_sprite_id + 16,hbar_x, -95, 75);
		jo_sprite_restore_sprite_scale();
		
		
		
		}else
		{
		jo_nbg2_printf(1, 1,  "TIME");
		jo_nbg2_printf(1, 3,  "%02d.%02d", players[target_player].mins,players[target_player].secs);
		}
		//laps
		//jo_sprite_enable_half_transparency();
		jo_sprite_draw3D(game.hud_sprite_id + 4,-140, 103, 100);
		jo_sprite_change_sprite_scale(2);
		if(players[target_player].laps>9)
		{		
		//divide laps by 10 to get first digit
		p1_laps = players[target_player].laps/10;
		jo_sprite_draw3D(game.hud_sprite_id + 5 + p1_laps,-100, 103, 100);
		//get modulus 10 to get second digit and move original sprite to right by 16
		p1_laps = players[target_player].laps % 10;		
		jo_sprite_draw3D(game.hud_sprite_id + 5 + p1_laps,-68, 103, 100);
		}else
		{
		p1_laps = players[target_player].laps;	
		jo_sprite_draw3D(game.hud_sprite_id + 5 + p1_laps,-100, 103, 100);
		}
		//positon
		jo_sprite_draw3D(game.hud_sprite_id + 5 + players[target_player].position,128, 103, 100);
		
		jo_sprite_restore_sprite_scale();
		//jo_sprite_disable_half_transparency();	
		
		//left side
		if(game.mode == GAMEMODE_TIMEATTACK)
		{
		jo_nbg2_printf(32, 1,  "BEAT");
		jo_nbg2_printf(32, 3,  "%02d.%02d", game.target_mins,game.target_secs);
		
		}else if(game.mode == GAMEMODE_1PLAYERSURVIVAL)
		{
		jo_nbg2_printf(1, 1,  "TIME");
		//time remaining
		
		players[target_player].mins = (game.target_time - players[target_player].current_time) / 60;
		players[target_player].secs = (game.target_time - players[target_player].current_time) % 60;
		
		jo_nbg2_printf(1, 3,  "%02d.%02d", players[target_player].mins,players[target_player].secs);
		
		if(game.target_time - players[target_player].current_time <= 0)
		{
		players[target_player].current_time = 0;
		transition_to_end_level();
		}
		
		}else
		{
		players[target_player].mins = players[target_player].best_time / 60;
		players[target_player].secs = players[target_player].best_time % 60;
		
		jo_nbg2_printf(32, 1,  "BEST");
		jo_nbg2_printf(32, 3,  "%02d.%02d", players[target_player].mins,players[target_player].secs);
		
			
		}
		
		
		
		
		}
		
			if(show_level_map)
			{
				//level map
				jo_sprite_set_palette(trackmap_pal.id);
				jo_sprite_change_sprite_scale(4);
				jo_sprite_draw3D(trackmap_tex,map_adj, 0, 75);
				jo_sprite_restore_sprite_scale();
				jo_sprite_set_palette(font_pal.id);
				
				for(int p = 0; p < game.players; p++)
				{
				
				map_x = (players[p].x / 48) + map_adj;
				map_y = (-players[p].z / 48);
				jo_sprite_draw3D_and_rotate(game.hud_sprite_id+19+p,map_x,map_y, 50,players[p].ry);
				}
			}
		
		}
		
		
		
		   
	
}

void				move_camera(jo_camera * current_cam, int p)

{
	//if(current_players == 1)
	//{
	bool reverse;	
	Sint16 rot_dif;
	
	if(cam_mode == 0)
	{
		
	///fixed camera
				players[p].cam_pitch = players[p].cam_pitch_adj;
				//players[p].cam_pitch = JO_DEG_TO_RAD(jo_atan2f((players[p].x - players[p].cam_pos_x),(players[p].z - players[p].cam_pos_z)))+players[p].cam_pitch_adj;
				//players[p].cam_pitch = slAtan(players[p].cam_dist, players[p].y-players[p].cam_pos_y);
				if(players[p].can_be_hurt)
				{
					//if(players[p].rear_cam)
					//{
					//players[p].cam_angle_y = (players[p].ary + players[p].cam_angle_adj+180);
					//}else
					//{
					players[p].cam_angle_y = players[p].cam_angle_adj;	
					//}
				}
				players[p].cam_horiz_dist = JO_DIV_BY_32768(players[p].cam_dist * jo_cos(players[p].cam_pitch));
				players[p].cam_vert_dist = JO_DIV_BY_32768(players[p].cam_dist * jo_sin(players[p].cam_pitch));
				
				players[p].cam_pos_x = JO_DIV_BY_32768((JO_MULT_BY_32768(players[p].x) + players[p].cam_horiz_dist * jo_sin(players[p].cam_angle_y)));
				players[p].cam_pos_z = JO_DIV_BY_32768((JO_MULT_BY_32768(players[p].z) + players[p].cam_horiz_dist * jo_cos(players[p].cam_angle_y)));
				
				players[p].cam_pos_y = players[p].y + players[p].cam_vert_dist - players[p].cam_height;
								
				players[p].cam_target_x = players[p].x;
				players[p].cam_target_y = players[p].y;// + players[p].cam_height;
				players[p].cam_target_z = players[p].z;	
				
		
	
		
	}else
	{
		if(cam_mode == 1)
		{
		players[p].cam_angle_adj = players[p].ary;		
		}
		
		if(cam_mode == 2)
		{
			
		///smooth follow
		//clamp
		if (players[p].cam_angle_adj > 180)
				players[p].cam_angle_adj -=360;
			else if (players[p].cam_angle_adj <= -180)
				players[p].cam_angle_adj +=360;
		
		rot_dif = players[p].cam_angle_adj - players[p].ary;
		
		if( rot_dif >= -180 && rot_dif < 180)
			{
			reverse = false;
			}else
			{
			reverse = true;	
			}
		
		
		if(JO_ABS(rot_dif)<=4)
		{
		players[p].cam_angle_adj = players[p].ary;
		}else
		{
			if(!reverse)
			{
				if(players[p].cam_angle_adj < players[p].ary)
				{
				players[p].cam_angle_adj += framerate;	
				}else if(players[p].cam_angle_adj > players[p].ary)
				{
				players[p].cam_angle_adj -= framerate;	
				}
			}else
			{
				if(players[p].cam_angle_adj < players[p].ary)
				{
				players[p].cam_angle_adj -= framerate;	
				}else if(players[p].cam_angle_adj > players[p].ary)
				{
				players[p].cam_angle_adj += framerate;	
				}
			}
		}
			
		}
	
		///cam behind player
				players[p].cam_pitch = players[p].cam_pitch_adj;
				//players[p].cam_pitch = JO_DEG_TO_RAD(jo_atan2f((players[p].x - players[p].cam_pos_x),(players[p].z - players[p].cam_pos_z)))+players[p].cam_pitch_adj;
				//players[p].cam_pitch = slAtan(players[p].cam_dist, players[p].y-players[p].cam_pos_y);
				if(players[p].can_be_hurt)
				{
					if(players[p].rear_cam)
					{
					players[p].cam_angle_y = (players[p].cam_angle_adj+180);
					}else
					{
					players[p].cam_angle_y = players[p].cam_angle_adj;	
					
					
					
					
					
					}
				}
				players[p].cam_horiz_dist = JO_DIV_BY_32768(players[p].cam_dist * jo_cos(players[p].cam_pitch));
				players[p].cam_vert_dist = JO_DIV_BY_32768(players[p].cam_dist * jo_sin(players[p].cam_pitch));
				
				players[p].cam_pos_x = JO_DIV_BY_32768((JO_MULT_BY_32768(players[p].x) + players[p].cam_horiz_dist * jo_sin(players[p].cam_angle_y)));
				players[p].cam_pos_z = JO_DIV_BY_32768((JO_MULT_BY_32768(players[p].z) + players[p].cam_horiz_dist * jo_cos(players[p].cam_angle_y)));
				
				players[p].cam_pos_y = players[p].y + players[p].cam_vert_dist - players[p].cam_height;
								
				players[p].cam_target_x = players[p].x;
				players[p].cam_target_y = players[p].y;// + players[p].cam_height;
				players[p].cam_target_z = players[p].z;	
		
		
		
	//}
		
		
	}	
		//camera zoom
		//int y_dist;
		
		
		switch(players[p].cam_zoom_num)
		{
		case 0:
				players[p].cam_zoom = CAM_DEFAULT_Y;
				players[p].cam_height = 50;
				players[p].cam_dist = -32;//24
				players[p].cam_pitch_adj = 24;
				break;
        case 1:
				players[p].cam_zoom = CAM_DEFAULT_Y - CAM_ZOOM_1;
				players[p].cam_height = 100;
				players[p].cam_dist = -48;
				players[p].cam_pitch_adj = 48;
				break;
				
		case 2:
				players[p].cam_zoom = CAM_DEFAULT_Y - CAM_ZOOM_2;
				players[p].cam_height = 150;
				players[p].cam_dist = -72;
				players[p].cam_pitch_adj = 72;
				break;
				
		case 3:
				players[p].cam_zoom = CAM_DEFAULT_Y - CAM_ZOOM_3;
				players[p].cam_height = 400;
				players[p].cam_dist = -90;
				players[p].cam_pitch_adj = 90;
				break;
				
		/*case 4:
				if(game.game_state == GAMESTATE_END_LEVEL)
				{
				players[p].cam_angle_y++;	
				players[p].cam_pitch = JO_DEG_TO_RAD(jo_atan2f(-(players[p].x - players[p].cam_pos_x),-(players[p].z - players[p].cam_pos_z)))+players[p].cam_pitch_adj;
				players[p].cam_horiz_dist = JO_DIV_BY_32768(players[p].cam_dist * jo_cos(players[p].cam_pitch));
				players[p].cam_vert_dist = JO_DIV_BY_32768(players[p].cam_dist * jo_sin(players[p].cam_pitch));
				players[p].cam_pos_x = -JO_DIV_BY_32768((JO_MULT_BY_32768(-players[p].x) + players[p].cam_horiz_dist * jo_sin(players[p].cam_angle_y)));
				players[p].cam_pos_z = -JO_DIV_BY_32768((JO_MULT_BY_32768(-players[p].z) + players[p].cam_horiz_dist * jo_cos(players[p].cam_angle_y)));
				players[p].cam_target_x = players[p].x;
				players[p].cam_target_y = players[p].y +players[p].cam_height ;
				players[p].cam_target_z = players[p].z;
				}
				break;*/
		}
		/*
		y_dist = players[p].cam_height - cam_zoom;
		
		if(y_dist >0)
		{
			players[p].cam_height -= CAM_SPEED;
		}else if(y_dist <0)
		{
			players[p].cam_height += CAM_SPEED;
		}
		
	*/
	
	//}
	
	//players[p].cam_angle_z  = -jo_atan2f((players[p].cam_pos_y - players[p].cam_target_y),(JO_ABS(players[p].cam_pos_x - players[p].cam_target_x) + JO_ABS(players[p].cam_pos_z - players[p].cam_target_z)));
	players[p].cam_angle_z = players[p].cam_pitch_adj;//slAtan(players[p].cam_horiz_dist, players[p].y - players[p].cam_pos_y);// + players[p].cam_pitch_adj;
	players[p].cam_angle_x = 0;
	
		
	jo_3d_camera_set_viewpoint(current_cam,players[p].cam_pos_x,players[p].cam_pos_y,players[p].cam_pos_z);
	jo_3d_camera_set_target(current_cam,players[p].cam_target_x,players[p].cam_target_y,players[p].cam_target_z);
	
		
	
}



void			    race_start(void)
{
	if (game.game_state != GAMESTATE_RACE_START)
       return;
   
   if (is_cd_playing)
        {
             CDDAStop();
            is_cd_playing = false;
        }
		
   //timer   
	
	if(game.race_start_timer < RACE_START_TIMER)
	{
	game.race_start_timer ++;	
	
		
	for(int p = 0; p < game.players; p++)
	{
		players[p].physics_speed = 0;
		players[p].next_waypoint = 1;
		
		
	}
		
		jo_sprite_change_sprite_scale(2);
		if(game.race_start_timer < 90)
		{
			
		//rotate camera
		if(players[0].cam_angle_adj == 0)
		{
		players[0].cam_angle_adj = 180;
		}
			
		//3
			if(game.race_start_timer > 60)
			{	
				if(game.race_start_timer < 70)
				{
				pcm_play(cpoint_sound, PCM_PROTECTED, 6);
				}
			jo_sprite_draw3D(game.hud_sprite_id + 8,0, 0, 100);
			}
		}else if(game.race_start_timer < 120)
		{
		//2
				if(game.race_start_timer < 100)
				{
				pcm_play(cpoint_sound, PCM_PROTECTED, 6);
				}
		jo_sprite_draw3D(game.hud_sprite_id + 7,0, 0, 100);
		}else if(game.race_start_timer < 150)
		{
		//1
				if(game.race_start_timer < 130)
				{
				pcm_play(cpoint_sound, PCM_PROTECTED, 6);
				}
		jo_sprite_draw3D(game.hud_sprite_id + 6,0, 0, 100);
		}
		else if(game.race_start_timer < 180)
		{
		//go!
				if(game.race_start_timer < 160)
				{
				pcm_play(pup_sound, PCM_PROTECTED, 6);
				}
		
		jo_sprite_draw3D(game.hud_sprite_id + 15,0, 0, 100);
		}
		jo_sprite_restore_sprite_scale();
		
		if(game.mode == GAMEMODE_2PLAYERVS)
		{
			slCurWindow(winNear);
			
			jo_sprite_change_sprite_scale(2);
			if(game.race_start_timer < 90)
			{
			
			//rotate camera
			if(players[1].cam_angle_adj == 0)
			{
			players[1].cam_angle_adj = 180;
			}
			
			//3
				if(game.race_start_timer > 60)
				{	
				jo_sprite_draw3D(game.hud_sprite_id + 8,0, 0, 100);
				}
			}else if(game.race_start_timer < 120)
			{
			//2
			jo_sprite_draw3D(game.hud_sprite_id + 7,0, 0, 100);
			}else if(game.race_start_timer < 150)
			{
			//1
			jo_sprite_draw3D(game.hud_sprite_id + 6,0, 0, 100);
			}
			else if(game.race_start_timer < 180)
			{
			//go!			
			jo_sprite_draw3D(game.hud_sprite_id + 15,0, 0, 100);
			}
			jo_sprite_restore_sprite_scale();
			slCurWindow(winFar);
			
			
		}
		
		
		
		
	}else
	{	
	//START RACE
	game.race_start_timer = 0;
	is_cd_playing = false;
	game.game_state = GAMESTATE_GAMEPLAY;
	for(int p = 0; p < game.players; p++)
	{	
	players[p].enable_controls = true;
	}
	}
	
	
 
}

void			    my_draw(void)
{
	if (game.game_state != GAMESTATE_GAMEPLAY && game.game_state != GAMESTATE_RACE_START)
       return;
	if(show_debug)
	{
	jo_nbg2_printf(12, 0, "*MICRO CIRCUIT MAYHEM*");
	}
	
    
	Sint16 x_dist;
	Sint16 y_dist;
	Sint16 z_dist;
	
	//play cd audio
	if(cd_track >0)
	{
	if (!is_cd_playing)
        {
             CDDAPlaySingle(cd_track, true);
            is_cd_playing = true;
        }
	}
	
	
	slCurWindow(winFar);

	move_camera(&cam1,target_player);
	
	slPushMatrix();
    {
			slRotX(DEGtoANG(players[target_player].cam_angle_z));
            slRotZ(DEGtoANG(players[target_player].cam_angle_x));
            slRotY(DEGtoANG(-players[target_player].cam_angle_y));
            slTranslate(toFIXED(-players[target_player].cam_pos_x), toFIXED(-players[target_player].cam_pos_y), toFIXED(-players[target_player].cam_pos_z));
			
			slPushMatrix();
			{
			if(use_light) computeLight();
			}
			slPopMatrix();
	
	for(Uint16 i = 0; i < total_sections; i++)
	{
		draw_map(&map_section[i],target_player);
	
	}
	
	for(int p = 0; p < game.players; p++)
	{
		if(players[p].alive)
		{
		draw_player(p);
		animate_player(p);
		player_collision_handling(p);
		
		set_player_position(p);
		
		}
	}
	//animate_player(0);
	//player_collision_handling(0);
	
	
	
		
	
	
	for(Uint16 w = 0; w < powerup_total; w++)
	{
	
	//set draw distance
	x_dist = JO_ABS(players[target_player].cam_pos_x - powerups[w].x);
	y_dist = JO_ABS(players[target_player].cam_pos_y - powerups[w].y);
	z_dist = JO_ABS(players[target_player].cam_pos_z - powerups[w].z);
	
		if((x_dist + y_dist + z_dist) < DRAW_DISTANCE)
		{
			//if(!powerups[w].used)
		//	{
			draw_powerups(&powerups[w]);
			//}
		}
	}
	
	if(game.level_inside == true)
		{
		draw_walls();
		}
	
	 }
	slPopMatrix();

	
	slCurWindow(winNear);
	//race_start();
	//jo_3d_camera_look_at(&cam2);
	//current_camera = cam2;
	if(game.mode == GAMEMODE_2PLAYERVS)
	{
	move_camera(&cam2,1);
		
	slPushMatrix();
    {
		
		slRotX(DEGtoANG(players[1].cam_angle_z));
        slRotZ(DEGtoANG(players[1].cam_angle_x));
        slRotY(DEGtoANG(-players[1].cam_angle_y));
        slTranslate(toFIXED(-players[1].cam_pos_x), toFIXED(-players[1].cam_pos_y), toFIXED(-players[1].cam_pos_z));
		
		slPushMatrix();
		{
        if(use_light) computeLight();
		}
		slPopMatrix();
		
	for(Uint16 i = 0; i < total_sections; i++)
	{
		draw_map(&map_section[i],1);
	
	}
	
	draw_player(1);
	//animate_player(1);
	//player_collision_handling(1);
	
	draw_player(0);
	
	
	
	for(Uint16 w = 0; w < powerup_total; w++)
	{
	
	//set draw distance
	x_dist = JO_ABS(players[1].cam_pos_x - powerups[w].x);
	y_dist = JO_ABS(players[1].cam_pos_y - powerups[w].y);
	z_dist = JO_ABS(players[1].cam_pos_z - powerups[w].z);
	
		if((x_dist + y_dist + z_dist) < DRAW_DISTANCE)
		{
			//if(!powerups[w].used)
		//	{
			draw_powerups(&powerups[w]);
			//}
		}
	}	
	
	if(game.level_inside == true)
		{
		draw_walls();
		}
	
	}
	slPopMatrix();
	}
	
	draw_hud();
	
	
	
	
	
   
   if(show_debug)
		{
		jo_nbg2_printf(0, 26, "POLYGON COUNT %4d" , jo_3d_get_polygon_count());
		jo_nbg2_printf(20, 1, "PPOS:\t%3d\t%3d\t%3d ",(int) players[target_player].x, (int) players[target_player].y, (int) players[target_player].z);
		jo_nbg2_printf(20, 2, "CPOS:\t%3d\t%3d ",(int) players[target_player].ary, (int) players[target_player].cam_angle_adj);
		//jo_nbg2_printf(20, 2, "RY TY \t%4d\t%4d",(int) players[target_player].ry, (int) players[target_player].tr );
		//jo_nbg2_printf(20, 2, "GCOUNT %3d",player_g_count);
		//jo_nbg2_printf(20, 2, "CLD :\t%3d\t%3d\t%3d ",(int) players[0].clouds->x, (int) players[0].clouds->y, (int) players[0].clouds->z);
		//jo_nbg2_printf(20, 2, "TOTAL MODELS   %d",model_total);
		//jo_nbg2_printf(20, 3, "TOTAL SECTIONS %d",total_sections);
		jo_nbg2_printf(0, 27, "POLYGONS DISPLAYED %4d" , jo_3d_get_displayed_polygon_count());
		//jo_nbg2_printf(0, 2, "total sections: %3d",total_map_sections);
		//jo_nbg2_printf(0, 5, "* DYNAMIC MEMORY USAGE: %d%%  ", jo_memory_usage_percent());
	jo_nbg2_printf(0, 4,  "SPRITE MEMORY USAGE: %d%%  ", jo_sprite_usage_percent());
	//jo_nbg2_printf(0, 5,  "PHYSICS SPEED 1: \t%3d\t%3d\t%3d  ", (int) players[0].physics_speed*100,(int) players[0].physics_speed_x_adj*100,(int) players[0].physics_grip*100);
	//jo_nbg2_printf(0, 6,  "PHYSICS SPEED 2: \t%3d\t%3d\t%3d  ", (int) players[1].physics_speed*100,(int) players[1].physics_speed_x_adj*100, (int) players[1].physics_grip*100);
		jo_nbg2_printf(0, 2, "L CP S W D:\t%2d\t%2d\t%2d\t%2d\t%4d",(int) players[target_player].laps, (int) players[target_player].current_checkpoint, (int) players[target_player].next_waypoint, players[target_player].current_waypoint, players[target_player].dist_to_next_waypoint);
		//jo_nbg2_printf(0, 3, "POS_POINTS %6d" , players[target_player].position_points);
		jo_nbg2_printf(0, 3, "POSITION %d" , players[target_player].position);
		}/*else
		{
		ztClearText();	
		}*/
    
}


void			load_car(int p, int model_number)
{
	
unsigned int total_point =0;
unsigned int total_polygon=0;
unsigned int total_attribute=0;
unsigned int total_normal=0;
Uint16		player_g_count = players[p].gstart;
Uint16 		tex;

XPDATA *players_car = (XPDATA *)car_data[p];
XPDATA *car_map = xpdata_[model_number];


		///points
		total_point = car_map->nbPoint;
		
		
		for (unsigned int i = 0; i<total_point; i++)
		   {
			
			players_car->pntbl[i][0] = car_map->pntbl[i][0];
			players_car->pntbl[i][1] = car_map->pntbl[i][1];
			players_car->pntbl[i][2] = car_map->pntbl[i][2];
			
			}
			players_car->nbPoint = total_point;

		///polygon
		total_polygon = car_map->nbPolygon;
		
		for (unsigned int j = 0; j<total_polygon; j++)
		   {
			players_car->pltbl[j].norm[0] = car_map->pltbl[j].norm[0];
			players_car->pltbl[j].norm[1] = car_map->pltbl[j].norm[1];
			players_car->pltbl[j].norm[2] = car_map->pltbl[j].norm[2];
			players_car->pltbl[j].Vertices[0] = car_map->pltbl[j].Vertices[0];
			players_car->pltbl[j].Vertices[1] = car_map->pltbl[j].Vertices[1];
			players_car->pltbl[j].Vertices[2] = car_map->pltbl[j].Vertices[2];
			players_car->pltbl[j].Vertices[3] = car_map->pltbl[j].Vertices[3];
						
			}
			players_car->nbPolygon = total_polygon;
			
		///attribute
		
		total_attribute = total_polygon;
		//g_count = 601;
		for (unsigned int k = 0; k<total_attribute; k++)
		   {			 
			players_car->attbl[k] = car_map->attbl[k];
			
		
			tex = (Uint16) car_map->attbl[k].texno;
			if(tex == 11)
			{
			tex = 10;
			players_car->attbl[k].texno = tex;	
			players_car->attbl[k].colno = LUTidx(tex);
			}else
			{
				switch(p)
				{
					case 0:	
							players_car->attbl[k].texno = tex;	
							players_car->attbl[k].colno = LUTidx(tex);
							break;
					case 1:	
							players_car->attbl[k].texno = tex + 33;	
							players_car->attbl[k].colno = LUTidx(tex + 132);//(4x33) - could be to do with data type?
							break;
					case 2:	
							players_car->attbl[k].texno = tex + 66;	
							players_car->attbl[k].colno = LUTidx(tex + 265);//(4x66) - could be to do with data type?
							break;
					case 3:	
							players_car->attbl[k].texno = tex + 99;	
							players_car->attbl[k].colno = LUTidx(tex + 396);//(4x99) - could be to do with data type?
							break;
				}
			
			
			
			}
			players_car->attbl[k].gstb = GRreal(player_g_count);	
			
			
			player_g_count ++;
			}
			
			///normal
			
			total_normal = total_polygon;
			for (unsigned int l = 0; l<total_normal; l++)
		   {
			 
			players_car->vntbl[l][0] = car_map->vntbl[l][0];
			players_car->vntbl[l][1] = car_map->vntbl[l][1];
			players_car->vntbl[l][2] = car_map->vntbl[l][2];
			
			}
}

void			load_binary(char * filename, void * startAddress)
{
char * stream;
 void * currentAddress;
unsigned int total_point =0;
unsigned int total_polygon=0;
unsigned int total_attribute=0;
unsigned int total_normal=0;
unsigned int total_collcubes=0;
int nxt = 0;

int line=2;
int length =0;
unsigned int att_tex = 0;
unsigned int att_plane = 0;
unsigned int att_flip = 0;
unsigned int g_count = 0;
unsigned int enemytype =0;
int enemyx =0;
int enemyy =0;
int enemyz =0;
int enemyxdist =0;
int enemyzdist =0;
unsigned int enemyspeed =0;
unsigned int enemyhits =0;
unsigned int puptype =0;
int pupx =0;
int pupy =0;
int pupz =0;
//unsigned int current_model = 0;
int section_type = 0;
int section_x = 0;
int section_y = 0;
int section_z = 0;
unsigned int att_meshOn = 0;

//reset total waypoints
total_waypoints = 0;

stream = jo_fs_read_file(filename, &length);
currentAddress = startAddress;

jo_nbg2_printf(0, line, "LOADING....:                    ");
///total models
model_total = jo_swap_endian_uint(*((unsigned int *)(stream)));
//jo_nbg2_printf(0, line, "MODEL_TOTAL:         %d     ", model_total);
//line++;
nxt +=4;

///main loop
for (unsigned int s = 0; s< model_total; s++)
	{
		
		///current_model
		xpdata_[s]= currentAddress;
		//current_model = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
		//jo_nbg2_printf(0, line, "CURRENT_MODEL:         %d     ", model_total);
		//line++;
		nxt +=4;

		///points
		total_point = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
		//jo_nbg2_printf(0, line, "LOADING....:                    ");
		nxt +=4;

		xpdata_[s]->pntbl = (POINT*)(xpdata_[s] + sizeof(unsigned int));

		for (unsigned int i = 0; i<total_point; i++)
		   {
			
			xpdata_[s]->pntbl[i][0] = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
			nxt +=4;
			xpdata_[s]->pntbl[i][1] = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
			nxt +=4;
			xpdata_[s]->pntbl[i][2] = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
			nxt +=4;
			
			//jo_printf(0, line, "point: {%d,%d,%d} %d",point1,point2,point3, i);
			//line++;
			
			   

			}
			xpdata_[s]->nbPoint = total_point;
			//jo_nbg2_printf(0, line, "TOTAL_POINT:         %d     ", total_point);
			//line++;

		///polygon

		total_polygon = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
		xpdata_[s]->pltbl = (POLYGON*)(xpdata_[s]->pntbl + sizeof(POINT)*total_point);
		//jo_nbg2_printf(0, line, "LOADING....:                    ");
		nxt +=4;

			for (unsigned int j = 0; j<total_polygon; j++)
		   {
			xpdata_[s]->pltbl[j].norm[0] = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
			nxt+=4;
			xpdata_[s]->pltbl[j].norm[1] = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
			nxt+=4;
			xpdata_[s]->pltbl[j].norm[2] = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
			nxt+=4;
			xpdata_[s]->pltbl[j].Vertices[0] = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
			nxt+=4;
			xpdata_[s]->pltbl[j].Vertices[1] = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
			nxt+=4;
			xpdata_[s]->pltbl[j].Vertices[2] = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
			nxt+=4;
			xpdata_[s]->pltbl[j].Vertices[3] = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
			nxt+=4;
			
			
			}
			xpdata_[s]->nbPolygon = total_polygon;
			//jo_nbg2_printf(0, line, "TOTAL_POLYGON:       %d     ", total_polygon);
			//line++;


		///attribute

		total_attribute = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
		xpdata_[s]->attbl = (ATTR*)(xpdata_[s]->pltbl + sizeof(POINT)*total_polygon);
	//	jo_nbg2_printf(0, line, "LOADING....:                    ");
		nxt +=4;
		
		/*if(filename == "CARS.BIN")
			{
			g_count = 600;
			}*/
		
			for (unsigned int k = 0; k<total_attribute; k++)
		   {
			 
			 att_tex = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
			nxt+=4;
			att_plane = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
			nxt+=4;
			att_meshOn = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
			nxt+=4;
			att_flip = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
			nxt+=4;
			
			if(filename == "CARS.BIN")
			{
			att_tex = PLAYER_TILESET+att_tex;
			//g_count = 0;
			}else
			{
			att_tex = MAP_TILESET+att_tex;
			}
			
			if(att_meshOn == 1)
			{
				if(att_plane == 1 && att_flip == 1)
				{
				ATTR bufAttr = ATTRIBUTE(Single_Plane, SORT_MAX, att_tex, LUTidx(att_tex), GRreal(g_count),Window_In|MESHon|HSSon|ECdis|CL32KRGB|CL_Gouraud, sprVflip, UseGouraud|UseNearClip);
				xpdata_[s]->attbl[k] = bufAttr;
				}else if(att_plane == 1)
				{
				ATTR bufAttr = ATTRIBUTE(Single_Plane, SORT_MAX, att_tex, LUTidx(att_tex), GRreal(g_count),Window_In|MESHon|HSSon|ECdis|CL32KRGB|CL_Gouraud, sprNoflip, UseGouraud|UseNearClip);
				xpdata_[s]->attbl[k] = bufAttr;		
				}else if(att_flip == 1)
				{
				ATTR bufAttr = ATTRIBUTE(Dual_Plane, SORT_MAX, att_tex, LUTidx(att_tex), GRreal(g_count),Window_In|MESHon|HSSon|ECdis|CL32KRGB|CL_Gouraud, sprVflip, UseGouraud|UseNearClip);
				xpdata_[s]->attbl[k] = bufAttr;		
				}else
				{
				ATTR bufAttr = ATTRIBUTE(Dual_Plane, SORT_MAX, att_tex, LUTidx(att_tex), GRreal(g_count),Window_In|MESHon|HSSon|ECdis|CL32KRGB|CL_Gouraud, sprNoflip, UseGouraud|UseNearClip);
				xpdata_[s]->attbl[k] = bufAttr;		
				}	
				
			}else
			{
				
				if(att_plane == 1 && att_flip == 1)
				{
				ATTR bufAttr = ATTRIBUTE(Single_Plane, SORT_MAX, att_tex, LUTidx(att_tex), GRreal(g_count),Window_In|MESHoff|HSSon|ECdis|CL32KRGB|CL_Gouraud, sprVflip, UseGouraud|UseNearClip);
				xpdata_[s]->attbl[k] = bufAttr;
				}else if(att_plane == 1)
				{
				ATTR bufAttr = ATTRIBUTE(Single_Plane, SORT_MAX, att_tex, LUTidx(att_tex), GRreal(g_count),Window_In|MESHoff|HSSon|ECdis|CL32KRGB|CL_Gouraud, sprNoflip, UseGouraud|UseNearClip);
				xpdata_[s]->attbl[k] = bufAttr;		
				}else if(att_flip == 1)
				{
				ATTR bufAttr = ATTRIBUTE(Dual_Plane, SORT_MAX, att_tex, LUTidx(att_tex), GRreal(g_count),Window_In|MESHoff|HSSon|ECdis|CL32KRGB|CL_Gouraud, sprVflip, UseGouraud|UseNearClip);
				xpdata_[s]->attbl[k] = bufAttr;		
				}else
				{
				ATTR bufAttr = ATTRIBUTE(Dual_Plane, SORT_MAX, att_tex, LUTidx(att_tex), GRreal(g_count),Window_In|MESHoff|HSSon|ECdis|CL32KRGB|CL_Gouraud, sprNoflip, UseGouraud|UseNearClip);
				xpdata_[s]->attbl[k] = bufAttr;		
				}	
				
				
			}
				if(filename != "CARS.BIN" && g_count>600)
				{
					g_count = 0;
				}
				else
				{
					g_count++;
				}
				
				if(filename == "CARS.BIN")
				{
				xpdata_[s]->attbl[k].atrb = Window_In|MESHoff|HSSon|ECdis|CL16Look|CL_Gouraud;
				}
				//XL2 wireframe mode
		//xpdata_[s]->attbl[k].dir &= ~FUNC_Texture;
		//xpdata_[s]->attbl[k].dir |= FUNC_PolyLine;
		//xpdata_[s]->attbl[k].dir |= FUNC_Polygon;
				
			}
			
			//jo_nbg2_printf(0, line, "TOTAL_ATTRIBUTE:       %d     ", total_attribute);
			//line++;
			
		///normal
		total_normal = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
		xpdata_[s]->vntbl = (VECTOR*)(xpdata_[s]->attbl + sizeof(POINT)*total_attribute);
		//jo_nbg2_printf(0, line, "LOADING....:                    ");
		nxt +=4;

			for (unsigned int l = 0; l<total_normal; l++)
		   {
			 
			xpdata_[s]->vntbl[l][0] = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
			nxt+=4;
			xpdata_[s]->vntbl[l][1] = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
			nxt+=4;
			xpdata_[s]->vntbl[l][2] = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
			nxt+=4;
			

			

			}
			//jo_nbg2_printf(0, line, "TOTAL_NORMAL:       %d     ", total_normal);
			//line++;
			
			
		//********************************LP FAR MODEL************************************
		
		currentAddress = (void*) (xpdata_[s]->vntbl + sizeof(VECTOR)*total_normal);
		pdata_LP_[s]= currentAddress;

		///lp point
		total_point = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
		//jo_nbg2_printf(0, line, "LOADING....:                    ");
		nxt +=4;

		pdata_LP_[s]->pntbl = (POINT*)(pdata_LP_[s] + sizeof(unsigned int));

		
		for (unsigned int i = 0; i<total_point; i++)
		{		
				
										
				pdata_LP_[s]->pntbl[i][0] = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
				nxt+=4;
				pdata_LP_[s]->pntbl[i][1] = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
				nxt+=4;
				pdata_LP_[s]->pntbl[i][2] = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
				nxt+=4;
				
				
		}

		pdata_LP_[s]->nbPoint = total_point;
		
		//jo_nbg2_printf(0, line, "TOTAL_POINT_LP:     %d     ", total_point);
		//line++;

		///lp polygon
		total_polygon = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
		//jo_nbg2_printf(0, line, "LOADING....:                    ");
		nxt +=4;

		pdata_LP_[s]->pltbl = (POLYGON*)(pdata_LP_[s]->pntbl + sizeof(POINT)*total_point);


		for (unsigned int j = 0; j<total_polygon; j++)
		{	
					
				pdata_LP_[s]->pltbl[j].norm[0] = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
				nxt+=4;
				pdata_LP_[s]->pltbl[j].norm[1] = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
				nxt+=4;
				pdata_LP_[s]->pltbl[j].norm[2] = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
				nxt+=4;
				pdata_LP_[s]->pltbl[j].Vertices[0] = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
				nxt+=4;
				pdata_LP_[s]->pltbl[j].Vertices[1] = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
				nxt+=4;
				pdata_LP_[s]->pltbl[j].Vertices[2] = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
				nxt+=4;
				pdata_LP_[s]->pltbl[j].Vertices[3] = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
				nxt+=4;
		}
		//jo_nbg2_printf(0, line, "TOTAL_POLY_LP:      %d     ", total_polygon);
		//line++;


		pdata_LP_[s]->nbPolygon = total_polygon;

		///lp attribute
		total_attribute = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
		//jo_nbg2_printf(0, line, "LOADING....:                    ");
		nxt +=4;

		pdata_LP_[s]->attbl = (ATTR*)(pdata_LP_[s]->pltbl + sizeof(POINT)*total_polygon);
		


			for (unsigned int k = 0; k<total_attribute; k++)
			{
				
				att_tex = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
				nxt+=4;
				att_plane = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
				nxt+=4;
				att_meshOn = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
				nxt+=4;
				att_flip = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
				nxt+=4;
				if(filename != "CARS.BIN")
				{
				att_tex = MAP_TILESET+att_tex;
				}
				
				if(att_meshOn == 1)
				{
					if(att_plane == 1 && att_flip == 1)
					{
					ATTR bufAttr = ATTRIBUTE(Single_Plane, SORT_MAX, att_tex, LUTidx(att_tex), GRreal(0),Window_In|MESHon|HSSon|ECdis|SPdis|CL32KRGB|CL_Gouraud, sprNoflip, UseDepth|UseNearClip);
					pdata_LP_[s]->attbl[k] = bufAttr;
					}else if(att_plane == 1)
					{
					ATTR bufAttr = ATTRIBUTE(Single_Plane, SORT_MAX, att_tex, LUTidx(att_tex), GRreal(0),Window_In|MESHon|HSSon|ECdis|SPdis|CL32KRGB|CL_Gouraud, sprNoflip, UseDepth|UseNearClip);
					pdata_LP_[s]->attbl[k] = bufAttr;		
					}else if(att_flip == 1)
					{
					ATTR bufAttr = ATTRIBUTE(Dual_Plane, SORT_MAX, att_tex, LUTidx(att_tex), GRreal(0),Window_In|MESHon|HSSon|ECdis|SPdis|CL32KRGB|CL_Gouraud, sprNoflip, UseDepth|UseNearClip);
					pdata_LP_[s]->attbl[k] = bufAttr;		
					}else
					{
					ATTR bufAttr = ATTRIBUTE(Dual_Plane, SORT_MAX, att_tex, LUTidx(att_tex), GRreal(0),Window_In|MESHon|HSSon|ECdis|SPdis|CL32KRGB|CL_Gouraud, sprNoflip, UseDepth|UseNearClip);
					pdata_LP_[s]->attbl[k] = bufAttr;		
					}
					
				}else
				{					
				
					if(att_plane == 1 && att_flip == 1)
					{
					ATTR bufAttr = ATTRIBUTE(Single_Plane, SORT_MAX, att_tex, LUTidx(att_tex), GRreal(0),Window_In|MESHoff|HSSon|ECdis|SPdis|CL32KRGB|CL_Gouraud, sprNoflip, UseDepth|UseNearClip);
					pdata_LP_[s]->attbl[k] = bufAttr;
					}else if(att_plane == 1)
					{
					ATTR bufAttr = ATTRIBUTE(Single_Plane, SORT_MAX, att_tex, LUTidx(att_tex), GRreal(0),Window_In|MESHoff|HSSon|ECdis|SPdis|CL32KRGB|CL_Gouraud, sprNoflip, UseDepth|UseNearClip);
					pdata_LP_[s]->attbl[k] = bufAttr;		
					}else if(att_flip == 1)
					{
					ATTR bufAttr = ATTRIBUTE(Dual_Plane, SORT_MAX, att_tex, LUTidx(att_tex), GRreal(0),Window_In|MESHoff|HSSon|ECdis|SPdis|CL32KRGB|CL_Gouraud, sprNoflip, UseDepth|UseNearClip);
					pdata_LP_[s]->attbl[k] = bufAttr;		
					}else
					{
					ATTR bufAttr = ATTRIBUTE(Dual_Plane, SORT_MAX, att_tex, LUTidx(att_tex), GRreal(0),Window_In|MESHoff|HSSon|ECdis|SPdis|CL32KRGB|CL_Gouraud, sprNoflip, UseDepth|UseNearClip);
					pdata_LP_[s]->attbl[k] = bufAttr;		
					}
				}
				//XL2 wireframe mode
		//pdata_LP_[s]->attbl[k].dir &= ~FUNC_Texture;
		//pdata_LP_[0]->attbl[k].dir |= FUNC_PolyLine;
		//pdata_LP_[s]->attbl[k].dir |= FUNC_Polygon;
				
			  
			}
		//jo_nbg2_printf(0, line, "TOTAL_ATTR_LP:      %d     ", total_attribute);
		//line++;
		
		//********************************************************************************

		///collpoints
		total_collcubes = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));

		currentAddress = (void*) (pdata_LP_[s]->attbl + sizeof(POINT)*total_attribute);
		cdata_[s]= currentAddress;
		cdata_[s]->cotbl = (COLLISON*)(cdata_[s] + sizeof(unsigned int));

		//jo_nbg2_printf(0, line, "LOADING....:                    ");
		nxt +=4;

			for (unsigned int m = 0; m<total_collcubes; m++)
		   {
			 
			cdata_[s]->cotbl[m].cen_x = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
			nxt+=4;
			cdata_[s]->cotbl[m].cen_y = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
			nxt+=4;
			cdata_[s]->cotbl[m].cen_z = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
			nxt+=4;
			cdata_[s]->cotbl[m].x_size = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
			nxt+=4;
			cdata_[s]->cotbl[m].y_size = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
			nxt+=4;
			cdata_[s]->cotbl[m].z_size = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
			nxt+=4;
			cdata_[s]->cotbl[m].att = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
			nxt+=4;
			
			//calculate ramp angle
			
			switch(cdata_[s]->cotbl[m].att)
			{
			case 1: cdata_[s]->cotbl[m].rot = jo_atan2f(cdata_[s]->cotbl[m].y_size , cdata_[s]->cotbl[m].z_size);
					
					break;
			case 2: 
					cdata_[s]->cotbl[m].rot = -jo_atan2f(cdata_[s]->cotbl[m].y_size , cdata_[s]->cotbl[m].x_size);
					break;
			case 3: cdata_[s]->cotbl[m].rot = -jo_atan2f(cdata_[s]->cotbl[m].y_size , cdata_[s]->cotbl[m].z_size);
					
					break;
			case 4: 
					cdata_[s]->cotbl[m].rot = jo_atan2f(cdata_[s]->cotbl[m].y_size , cdata_[s]->cotbl[m].x_size);
					break;	
			
			default: cdata_[s]->cotbl[m].rot = 0;
					
					break;
			
			
				
				
			}
			
			//record checkpoint positions
			if(cdata_[s]->cotbl[m].att >= 10 && cdata_[s]->cotbl[m].att <=99)
			{
			create_checkpoint(&checkpoints[cdata_[s]->cotbl[m].att - 10], s, cdata_[s]->cotbl[m].cen_x,cdata_[s]->cotbl[m].cen_y,cdata_[s]->cotbl[m].cen_z,0);			
			}
			
			//record waypoint positions
			if(cdata_[s]->cotbl[m].att >= 100 && cdata_[s]->cotbl[m].att <=199)
			{
			create_waypoint(&waypoints[cdata_[s]->cotbl[m].att - 100], s, cdata_[s]->cotbl[m].cen_x,cdata_[s]->cotbl[m].cen_y,cdata_[s]->cotbl[m].cen_z);			
			}
			
			
			}
			
			cdata_[s]->nbCo = total_collcubes;
			//jo_nbg2_printf(0, line, "TOTAL_COLLCUBES:      %d     ", total_collcubes);
			//line++;
	
			currentAddress = (void*) (cdata_[s]->cotbl + sizeof(COLLISON)*total_collcubes);
		
			
	
	}
	
	
///create map layout

		total_sections = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
		nxt+=4;
		
		for (unsigned int n = 0; n<total_sections; n++)
			{
		
			
		section_type = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
		nxt+=4;
		section_x = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
		nxt+=4;
		section_y = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
		nxt+=4;
		section_z = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
		nxt+=4;
		
		create_map_section(&map_section[n], section_type,section_x,section_y,section_z);	
		
			}
			
		game.start_x = checkpoints[0].x + map_section[checkpoints[0].section].x;
		game.start_z = checkpoints[0].z + map_section[checkpoints[0].section].z;
			
		//jo_nbg2_printf(0, line, "TOTAL_SECTIONS:         %d, g:%d     ", total_sections,g_count);
		//line++;


	
	
///ADD ENEMIES
	

//jo_nbg2_printf(0, line, "LOADING....:                    ");

enemy_total = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
nxt +=4;

	for (int e = 0; e<enemy_total; e++)
		{
		
		enemytype = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
		nxt+=4;
		enemyx = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
		nxt+=4;
		enemyy = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
		nxt+=4;
		enemyz = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
		nxt+=4;
		enemyxdist = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
		nxt+=4;
		enemyzdist = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
		nxt+=4;
		enemyspeed = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
		nxt+=4;
		enemyhits = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
		nxt+=4;

		create_enemy(&enemies[e], enemytype, enemyx,enemyy, enemyz, enemyxdist,enemyzdist,enemyspeed,enemyhits);
		
		
		}
		
//jo_nbg2_printf(0, line, "ENEMY_TOTAL:         %d     ", enemy_total);
//line++;
	
//POWERUPS
//jo_nbg2_printf(0, line, "LOADING....:                    ");

powerup_total = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
nxt +=4;

		for (int w = 0; w<powerup_total; w++)
		{
				
		puptype = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
		nxt+=4;
		pupx = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
		nxt+=4;
		pupy = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
		nxt+=4;
		pupz = jo_swap_endian_uint(*((unsigned int *)(stream+nxt)));
		nxt+=4;
		create_powerup(&powerups[w],puptype,pupx, pupy, pupz);
		
		}
	//jo_nbg2_printf(0, line, "POWERUP_TOTAL:         %d     ", powerup_total);
//line++;

	line = 0;
	ztClearText();

	jo_free(stream);

}

void			load_textures(char * filename, int total_tiles)
{

	jo_sprite_free_from(game.map_sprite_id);
	game.map_sprite_id = jo_sprite_add_tga_tileset("TEX", filename,JO_COLOR_Red,MAP_Tileset,total_tiles);
	
			
	
}

void			load_level(void)
{
	
	load_textures(level_data[game.level].tileset,44);
	load_binary((char*)level_data[game.level].file_name, (void*)WORK_RAM_LOW);
	game.level_inside = level_data[game.level].is_inside;
	cd_track = level_data[game.level].cd_track;
	
	//may add survival mode at some point
	if(game.level >=50)
	{
	load_textures("WB.TGA",44);
	load_binary((char*)"SV1.BIN", (void*)WORK_RAM_LOW);
	game.level_inside = true;
	cd_track = 0;	
	game.target_mins = survival_target_time[game.level-50] / 60;
	game.target_secs = survival_target_time[game.level-50] % 60;
	game.target_time = survival_target_time[game.level-50];
	}else
	{
	game.target_mins = level_data[game.level].level_target_time / 60;
	game.target_secs = level_data[game.level].level_target_time % 60;
	}
	
}

void            load_preview(char * filename)
{
	jo_sprite_free_from(preview_tex);  
    jo_set_tga_default_palette(&preview_pal);   
	preview_tex = jo_sprite_add_tga("BG", filename, JO_COLOR_Transparent);
		
}

void            load_trackmap(char * filename)
{
	jo_sprite_free_from(trackmap_tex);  
    jo_set_tga_default_palette(&trackmap_pal);   
	trackmap_tex = jo_sprite_add_tga("BG", filename, JO_COLOR_Transparent);	
	
}

void	set_palette(Uint16 * palette, Uint16 TextureId)
{
    //For SGL only use slDMACopy instead : slDMACopy( (void*)palette, (void*)adr, sizeof(palette) );
     //slDMACopy( (void*)palette, (void*)returnLUTaddr(TextureId), sizeof(palette) );
	jo_dma_copy(palette, (void*)(returnLUTaddr(TextureId)), sizeof(Uint16)*16);

    //Only if you want to use the VDP2 CRAM for your sprite, else skip this
    //jo_palette_to_cram(palette, (void*)(returnCRAMaddr(TextureId)), 16);
}

void				change_player_palette(Uint8 p, Uint8 pal_num, Uint16 colour)
{
	
	switch(p)
	{
			case 0:	P1_S_PALETTE[pal_num] = colour;
					for(int tex_num = 0; tex_num < 33; tex_num++)
					{	
					img_4bpp *current_texture=(img_4bpp *)player1_img_data[tex_num];
					set_palette(current_texture->palette, PLAYER_TILESET+tex_num);
					}
					break;
					
			case 1:	P2_S_PALETTE[pal_num] = colour;
					for(int tex_num = 0; tex_num < 33; tex_num++)
					{	
					img_4bpp *current_texture=(img_4bpp *)player2_img_data[tex_num];
					set_palette(current_texture->palette, PLAYER2_TILESET+tex_num);
					}
					break;
					
			case 2:	P3_S_PALETTE[pal_num] = colour;
					for(int tex_num = 0; tex_num < 33; tex_num++)
					{	
					img_4bpp *current_texture=(img_4bpp *)player3_img_data[tex_num];
					set_palette(current_texture->palette, PLAYER3_TILESET+tex_num);
					}
					break;
					
			case 3:	P4_S_PALETTE[pal_num] = colour;
					for(int tex_num = 0; tex_num < 33; tex_num++)
					{	
					img_4bpp *current_texture=(img_4bpp *)player4_img_data[tex_num];
					set_palette(current_texture->palette, PLAYER4_TILESET+tex_num);
					}
					break;
					
			default:P1_S_PALETTE[pal_num] = colour;
					for(int tex_num = 0; tex_num < 33; tex_num++)
					{	
					img_4bpp *current_texture=(img_4bpp *)player1_img_data[tex_num];
					set_palette(current_texture->palette, PLAYER_TILESET+tex_num);
					}
					break;
		
	}
	
	
	
}

void 			transition_to_title_screen(void)
{
		pcm_play(players[0].horn_sound, PCM_SEMI, 6);
		is_cd_playing = false;
		game.level=0;
		load_level();
		ztClearText();
		init_1p_display();
		current_players = 0;
		game.game_state = GAMESTATE_TITLE_SCREEN;
}

void 			transition_to_player_select(void)
{
	ztClearText();
	load_binary((char*)"CARS.BIN", (void*)WORK_RAM_LOW);
	for(int p = 0; p < game.players; p++)
	{
		players[p].cam_zoom_num = 0;
	players[0].cam_angle_x = 0;
	players[0].cam_angle_y = 0;
	players[0].cam_angle_z = 0;
	players[0].cam_pos_x = 0;
	players[0].cam_pos_y = 0;
	players[0].cam_pos_z = 0;
	players[0].ry = 0;
	players[0].x = 0;
	players[0].y = 0;
	players[0].z = 0;
	
	
	load_car(p,0);
	players[p].colour1 = default_colour[0][p*3];
	players[p].colour2 = default_colour[0][(p*3)+1];
	players[p].colour3 = default_colour[0][(p*3)+2];
			
	change_player_palette(p, 1, JO_COLOR_RGB(colour_options[players[p].colour1][0],colour_options[players[p].colour1][1],colour_options[players[p].colour1][2]));
	change_player_palette(p, 2, JO_COLOR_RGB(colour_options[players[p].colour2][0],colour_options[players[p].colour2][1],colour_options[players[p].colour2][2]));
	change_player_palette(p, 13, JO_COLOR_RGB(shadow_colour[players[p].colour3][0],shadow_colour[players[p].colour3][1],shadow_colour[players[p].colour3][2]));
	}
	game.game_state = GAMESTATE_PLAYER_SELECT;
}



void 			transition_to_level_select(void)
{
	ztClearText();
	init_1p_display();
	jo_disable_background_3d_plane(JO_COLOR_Black);
	jo_clear_background	(JO_COLOR_Black);
	game.level=0;
	load_level();
	if(game.mode == GAMEMODE_1PLAYERSURVIVAL)
	{
	load_preview(survival_preview[0]);
	}else
	{
		if(game.select_level == 0)
		{
		load_preview(level_data[1].level_preview);	
		load_trackmap(level_data[1].level_map);	
		}else
		{
		load_preview(level_data[game.select_level].level_preview);		
		load_trackmap(level_data[game.select_level].level_map);	
		}
	}
	
	players[0].cam_angle_x = 0;
	players[0].cam_angle_y = 0;
	players[0].cam_angle_z = 0;
	players[0].cam_pos_x = 0;
	players[0].cam_pos_y = 0;
	players[0].cam_pos_z = 0;
	players[0].ry = 0;
	players[0].x = 0;
	players[0].y = 0;
	players[0].z = 0;
	
	game.game_state = GAMESTATE_LEVEL_SELECT;
}

void			clear_level(void)
{
	game.game_state = GAMESTATE_UNINITIALIZED;
	jo_sprite_free_from(game.map_sprite_id);
	//jo_sprite_free_from(game.hud_sprite_id);
	for(int p = 0; p < game.players; p++)
	{
	stop_sounds(p);
	}
	//stop_sounds(1);
	is_cd_playing = false;
	transition_to_level_select();	
}

int menu(char **current_menu,int menu_total,int x, int y,int menu_num)
{
int max_menu_size_x = 0;
int current_menu_size = 0;
int bg_x;
int bg_y;
int max_menu_size_y;

bg_x = (x*8)-16;
bg_y = (y*8)-8;
max_menu_size_y = menu_total+2;

for(int m = 0; m < menu_total; m++)
	{
	jo_nbg2_printf(x, y+m, "%s",current_menu[m]);
	
	//get length of longest menu item
	current_menu_size = count_char((char*)current_menu[m]);
		if(max_menu_size_x < current_menu_size)
		{
		max_menu_size_x = current_menu_size;
		}
	
	}
	max_menu_size_x +=4;
	
	jo_nbg2_printf(x-1, y+menu_num, ">");
	
	//draw bg sprites based on number of menu items and longest menu item
	for(int bg_row = 0; bg_row < max_menu_size_y; bg_row++)
	{
		
		for(int bg_col = 0; bg_col < max_menu_size_x; bg_col++)
		{
		//render_CLUT_sprite2(11,bg_x+(bg_col*8), bg_y +(bg_row*8), 200);
			if(bg_row == 0 || bg_row == max_menu_size_y-1 || bg_col == 0 || bg_col == max_menu_size_x-1)
			{
			//draw border
			jo_sprite_draw3D2(game.hud_sprite_id+18,bg_x+(bg_col*8), bg_y +(bg_row*8), 100);
			}else
			{
			jo_sprite_draw3D2(game.hud_sprite_id+18,bg_x+(bg_col*8), bg_y +(bg_row*8), 100);	
			}
		}
	}
	
	
	if (KEY_DOWN(0,PER_DGT_KU))
	{
		pcm_play(cpoint_sound, PCM_SEMI, 6);           
			
        if (menu_num == 0)
		{
			menu_num = menu_total-1;
		}
        else
		{
			menu_num --;
		}
		ztClearText();
	}
 
	if (KEY_DOWN(0,PER_DGT_KD))
    {
    	pcm_play(cpoint_sound, PCM_SEMI, 6);

        if (menu_num == menu_total-1)
		{
            menu_num = 0;
		}else
		{
			menu_num ++;
		}
        ztClearText();
    }
	
return menu_num;	
}

void			    object_viewer(void)
{
	if (game.game_state != GAMESTATE_OBJECT_VIEWER)
		
       return;
   
	XPDATA * current_object;
	Uint32 nbPt;
	
	jo_nbg2_printf(0, 0, "              OBJECT VIEWER");
	jo_nbg2_printf(0, 2, "MODEL TYPE: %2d", map_builder_mode);
	jo_nbg2_printf(0, 3, "POLYGON: %3d", object_pol_num);
	jo_nbg2_printf(20, 2, "MODEL NUMBER: %2d", object_number);
	jo_nbg2_printf(0, 28, "Y TO CHANGE TYPE, B TO CHANGE OBJ NUM,  ");
	jo_nbg2_printf(0, 29, "C TO SHOW POLYGON, A TO CHANGE POLYGON  ");
	
	
	
   
   jo_3d_camera_look_at(&cam1);
	
	
	object_number = map_builder_model;	
	current_object= xpdata_[map_builder_model];
   /*switch(map_builder_mode)
			{
			case 0:
					object_number = map_builder_model;	
					current_object= xpdata_[map_builder_model];
						
					break;
			
			case 1:
					players[0].can_be_hurt = true;
					current_object= (XPDATA *)car_data[map_builder_car];			
					break;
					
			case 2:
					object_number = map_builder_powerup;	
					current_object=(XPDATA *)pup_data[map_builder_powerup];				
					break;
			}*/
   
	nbPt = current_object->nbPolygon;
   
   
   map_builder_draw_section();
   
 /*  switch(map_builder_mode)
		{
        case 0:
				map_builder_draw_section();
				break;
				
		case 1:
				draw_player(map_builder_car);
				break;
				
		case 2:
				map_builder_draw_powerup();
				break;		
		
		}*/
   
	
	if (!jo_is_pad1_available())
		return;
	
	if (KEY_PRESS(0,PER_DGT_KU))
		{
		//rotate x	
		players[0].rx--;
		}
			
	if (KEY_PRESS(0,PER_DGT_KD))
		{
		//rotate -x	
		players[0].rx++;		
		}
	
	if (KEY_PRESS(0,PER_DGT_KL))
		{
		//rotate -y	
		players[0].ry--;
		}
		
	if (KEY_PRESS(0,PER_DGT_KR))
		{//rotate y		
		players[0].ry++;
		}	
		
		
	if (KEY_PRESS(0,PER_DGT_TL))
		{//zoom out			
		if(object_scale >0.02f)
			object_scale-=0.02f;
		}
		
	if (KEY_PRESS(0,PER_DGT_TR))
		{
		//zoom in	
		if(object_scale <2.0f)
			object_scale+=0.05f;		
		}
		
	if (KEY_DOWN(0,PER_DGT_TB))
		{
			
			if(object_show_poly)
					{
						current_object->attbl[object_pol_num].texno = object_last_texture;
						current_object->attbl[object_pol_num].colno = LUTidx(object_last_texture);
						object_show_poly = false;
					}
			object_pol_num = 0;
			
			switch(map_builder_mode)
			{
			case 0:	++map_builder_model;
					if (map_builder_model >= model_total)
					{
					map_builder_model = 0;	
					}  
					break;
			
			case 1:	++map_builder_car;
					if (map_builder_car >= MAX_PLAYERS)
					{
					map_builder_car = 0;			
					}  
					break;
					
			case 2:	++map_builder_powerup;
					if (map_builder_powerup >= 7)
					{
					map_builder_powerup = 0;			
					}  
					break;
			}
			      
		}
	
	if (KEY_DOWN(0,PER_DGT_TY))
		{
			
			if(object_show_poly)
					{
						current_object->attbl[object_pol_num].texno = object_last_texture;
						current_object->attbl[object_pol_num].colno = LUTidx(object_last_texture);
						object_show_poly = false;
					}
			object_pol_num = 0;
			
			if(map_builder_mode >=2)
			{
			map_builder_mode=0;
			}else
			{
			map_builder_mode++;
			}
		}
		
	if (KEY_DOWN(0,PER_DGT_TA) && object_show_poly)
		{
			object_last_pol_num = object_pol_num;
			
			if(object_pol_num == nbPt-1)
			{
			object_pol_num = 0;
			}else
			{
			object_pol_num++;
			}
			
			current_object->attbl[object_last_pol_num].texno = object_last_texture;
			current_object->attbl[object_last_pol_num].colno = LUTidx(object_last_texture);
			object_last_texture = (Uint16) current_object->attbl[object_pol_num].texno;
			current_object->attbl[object_pol_num].texno = MAP_TILESET+19;
			current_object->attbl[object_pol_num].colno = LUTidx(MAP_TILESET+19);
			
		}
		
	if (KEY_DOWN(0,PER_DGT_TC))
		{
			if(object_show_poly)
			{
				current_object->attbl[object_pol_num].texno = object_last_texture;
				current_object->attbl[object_pol_num].colno = LUTidx(object_last_texture);
				object_show_poly = false;
			}else
			{
				object_show_poly = true;
				object_pol_num = 0;
				object_last_texture = (Uint16) current_object->attbl[object_pol_num].texno;
				object_last_pol_num = object_pol_num;
				current_object->attbl[object_pol_num].texno = MAP_TILESET+19;
				current_object->attbl[object_pol_num].colno = LUTidx(MAP_TILESET+19);
			}
		}
		
	jo_nbg2_printf(0, 4, "POLYS: %3d", jo_3d_get_polygon_count());
		jo_nbg2_printf(20, 4, "TEX: %3d", object_last_texture - MAP_TILESET);
		
		jo_nbg2_printf(0, 5, "CC: %2d", map_section[map_builder_model].a_cdata->nbCo);
	
	 // did player one pause the game?
    if (KEY_DOWN(0,PER_DGT_ST))
		{
			if(game.pressed_start == false)
			{
				object_scale =1;
				players[0].rx = 0;
				players[0].ry = 0;
				game.game_state = GAMESTATE_EXTRA_SELECT;
				jo_sprite_free_from(game.map_sprite_id);
				ztClearText();
				game.level=0;
				load_level();
				game.level=1;
			}
			game.pressed_start = true;
		}
    else
		{
			game.pressed_start = false;
		}
		
		
    
		
}

void			    pause_game(void)
{
	if (game.game_state != GAMESTATE_PAUSED)
       return;
   
   
   //menu
	jo_nbg2_printf(16, 8, "PAUSED");
	game.pause_menu = menu((char**)menu_pause,PAUSE_MENU_MAX,12,12,game.pause_menu);
	jo_nbg2_printf(12, 15, "MUSIC VOL %2d",music_vol);
	jo_nbg2_printf(12, 16, "CAMERA %s",cam_modes[cam_mode]);
	jo_nbg2_printf(12, 17, "SHOW MAP %d",show_level_map);
   
   //draw screen still
   
   jo_3d_camera_look_at(&cam1);
 	
	for(int i = 0; i < total_sections; i++)
	{
	draw_map(&map_section[i],0);
	}
	
	for(int p = 0; p < game.players; p++)
	{
		if(players[p].alive)
		{
		draw_player(p);
		}
	}
      
    slCurWindow(winNear);
	draw_hud();

	
if (game.pause_menu == 3)
    {
		if (KEY_DOWN(0,PER_DGT_KR))
		{
			if(music_vol < 7)
			{
			music_vol += 1;
			CDDASetVolume(music_vol);
			}
			
		}else if (KEY_DOWN(0,PER_DGT_KL))
		{
			if(music_vol > 0)
			{
			music_vol -= 1;
			CDDASetVolume(music_vol);
			}
		}
	
	}
	
	if (game.pause_menu == 4)
    {
		if (KEY_DOWN(0,PER_DGT_KR))
		{
			ztClearText();	
			if(cam_mode < 2)
			{
			cam_mode += 1;
			}
			
		}else if (KEY_DOWN(0,PER_DGT_KL))
		{
			ztClearText();	
			if(cam_mode > 0)
			{
			cam_mode -= 1;
			}
		}
	
	}
	
	if (game.pause_menu == 5)
    {
		if (KEY_DOWN(0,PER_DGT_KR) || KEY_DOWN(0,PER_DGT_KL))
		{
			show_level_map ^= true;
			
		}
	
	}
	
    // did player one pause the game?
   if (KEY_DOWN(0,PER_DGT_ST))
    {
        if(game.pressed_start == false)
        {
            ztClearText();
			
			switch(game.pause_menu)
			{
				case 0: for(int p = 0; p < game.players; p++)
						{
						players[p].pause_time += getSeconds() - game.pause_start_time;
						}
						game.game_state = GAMESTATE_GAMEPLAY;
						pcm_play(pup_sound, PCM_SEMI, 6);
						break;
						
				case 1: for(int p = 0; p < game.players; p++)
						{
						players[p].pause_time += getSeconds() - game.pause_start_time;
						}
						game.game_state = GAMESTATE_GAMEPLAY;
						reset_to_last_checkpoint(0);	
						break;
						
				case 2: game.game_state = GAMESTATE_GAMEPLAY;
						reset_demo();
						break;
						
				case 3: pcm_play(pup_sound, PCM_SEMI, 6);
						break;
				
				case 4: pcm_play(pup_sound, PCM_SEMI, 6);
						break;
						
				case 5: pcm_play(pup_sound, PCM_SEMI, 6);
						break;
						
				case 6: game.game_state = GAMESTATE_UNINITIALIZED;
						clear_level();
						break;
										
				
			}
			
        
        }
        game.pressed_start = true;
    }
    else
    {
        game.pressed_start = false;
    }
	
		slCurWindow(winFar);
}

void			attract_mode(void)
{
// random car selection for 4 players

// random level selection and load

//set timer
//set attract_mode flag to true
//go to 1 player race - add somewhere if timer runs out or start is pressed or race ends, return to title screen and reset title screen timer and set attract mode flag to false
}

void            title_screen(void)
{
	
	if (game.game_state != GAMESTATE_TITLE_SCREEN)
    return;

	jo_nbg2_printf(34, 28, "V0.3");

	if (!is_cd_playing)
        { 
			CDDAPlaySingle(2, true);
            is_cd_playing = true;
        }
	
	move_camera(&cam1,0);
	slPushMatrix();
    {
        if(use_light) computeLight();
     		
    }
	slPopMatrix();
	
	//logo
	jo_sprite_draw3D(MAP_TILESET+2,-72, -48, 100);
	jo_sprite_draw3D(MAP_TILESET+3,-24, -48, 100);
	jo_sprite_draw3D(MAP_TILESET+4,24, -48, 100);
	jo_sprite_draw3D(MAP_TILESET+5,72, -48, 100);
	
	jo_sprite_draw3D(MAP_TILESET+6,-72, 0, 100);
	jo_sprite_draw3D(MAP_TILESET+7,-24, 0, 100);
	jo_sprite_draw3D(MAP_TILESET+8,24, 0, 100);
	jo_sprite_draw3D(MAP_TILESET+9,72, 0, 100);
	
	//background model	

	slPushMatrix();
	{
	slRotX(DEGtoANG(players[0].cam_angle_z));
    slRotZ(DEGtoANG(players[0].cam_angle_x));
    slRotY(DEGtoANG(-players[0].cam_angle_y));
    slTranslate(toFIXED(-players[0].cam_pos_x), toFIXED(-players[0].cam_pos_y), toFIXED(-players[0].cam_pos_z));
	
		slPushMatrix();
		{
		slTranslate(toFIXED(map_section[0].x), toFIXED(map_section[0].y), toFIXED(map_section[0].z+100));
		slRotY(DEGtoANG(players[0].effect_ry));
		slPutPolygonX(map_section[0].map_model, light);
		}
		slPopMatrix();
				
	}
	slPopMatrix();
				
	players[0].effect_ry ++;
	
	if (players[0].effect_ry > 180)
		players[0].effect_ry -=360;
	else if (players[0].effect_ry <= -180)
		players[0].effect_ry +=360;
	
		
	//menu
	game.title_screen_menu = menu((char**)menu_titlescreen,TITLE_SCREEN_MENU_MAX,14,22,game.title_screen_menu);
	
	
	if (KEY_DOWN(0,PER_DGT_ST))
	{	
		pcm_play(pup_sound, PCM_SEMI, 6);
		game.game_state = GAMESTATE_UNINITIALIZED;
		ztClearText();
		game.pressed_start = true;
		
		switch(game.title_screen_menu)
		{
		case GAMEMODE_2PLAYERVS:	game.players = 2;
									current_players = 2;
									break;
		
		case GAMEMODE_1PLAYERRACE:	game.players = 4;
									current_players = 1;
									break;
		
		case GAMEMODE_1PLAYERSURVIVAL:	game.players = 4;
										current_players = 1;
									break;
				
		default: 					game.players = 1;	
									current_players = 1;
			
		}	
		
		game.mode = game.title_screen_menu;
		create_player();
		
		if(game.mode == GAMEMODE_2PLAYERVS)
		{
			init_2p_display();	
		}
		
		transition_to_player_select();
	}	 
	
	
}


void my_vblank()
{

    if(enableRTG == 1)
        slGouraudTblCopy();
}

void init_display(void)
{	
	jo_core_init(JO_COLOR_Black);
   
	slSetDepthLimit(0,11,5);
	slSetDepthTbl(DepthDataBlack,0xf000,32);
	
	slInitGouraud(gourRealMax, GOUR_REAL_MAX, GRaddr, vwork);
	slIntFunction(my_vblank);
	/**Set your color here if you need one (depending on your light source)**/
    slSetGouraudColor(CD_White);
				
}


void		cpu_control(void)
{
	if (game.game_state != GAMESTATE_GAMEPLAY )
       return;
    if(game.mode != GAMEMODE_1PLAYERRACE && game.mode != GAMEMODE_1PLAYERSURVIVAL)
	   return;
   
    Sint16 rot_dif = 0;
    bool reverse = false;


	for(int p = current_players; p < game.players; p++)
	{
			
			players[p].cpu_left = false;
			players[p].cpu_right = false;	
			
			if(game.mode == GAMEMODE_1PLAYERSURVIVAL)
			{
			//target player
			players[p].tx = players[0].x;
			players[p].ty = players[0].y;
			players[p].tz = players[0].z;
			
			}else
			{
			//target waypoint
			players[p].tx = waypoints[players[p].next_waypoint].x + map_section[waypoints[players[p].next_waypoint].section].x;
			players[p].ty = waypoints[players[p].next_waypoint].y + map_section[waypoints[players[p].next_waypoint].section].y;
			players[p].tz = waypoints[players[p].next_waypoint].z + map_section[waypoints[players[p].next_waypoint].section].z;
			}		
			
			
			players[p].tr = jo_atan2f((players[p].tx - players[p].x),(players[p].tz - players[p].z)) ;
			if (players[p].tr > 180)
				players[p].tr -=360;
			else if (players[p].tr <= -180)
				players[p].tr +=360;
			
			rot_dif = (players[p].ry - players[p].tr);
			
			if( rot_dif >= -180 && rot_dif < 180)
			{
			reverse = false;
			}else
			{
			reverse = true;	
			}
				
			if( rot_dif <= 6 && rot_dif >= -6)
			{
			//players[p].ry = players[p].tr;
			}else
			{
				if(!reverse)
				{
					if(players[p].ry < players[p].tr)
					{
					players[p].cpu_right = true;
						
					}else if (players[p].ry > players[p].tr)
					{
					players[p].cpu_left = true;
						
					}
				}else
				{
					if(players[p].ry < players[p].tr)
					{
					players[p].cpu_left = true;
						
					}else if (players[p].ry > players[p].tr)
					{
					players[p].cpu_right = true;
						
					}
					
				}			
		
			}	
		
		
			players[p].cpu_gas = true;
				
			if(players[p].current_powerup > 0 && players[p].cpu_pressed_action == false)
			{
			players[p].cpu_action = true;	
			players[p].cpu_pressed_action = true;	
			}else
			{
			players[p].cpu_action = false;
			if(players[p].current_powerup == 0)
			{
			players[p].cpu_pressed_action = false;
			}
			}
				
			rot_dif = 0.0f;
			reverse = false;
				
			players[p].cpu_siren = true;
			
		
			
			
		if(!players[p].enable_controls)
		{
		players[p].cpu_gas = false;	
		players[p].cpu_pressed_action = false;
		players[p].cpu_action = false;
		players[p].cpu_siren = false;
		players[p].cpu_left = false;
		players[p].cpu_right = false;
		}
	
		
	}
	
	
}



void			my_gamepad(void)
{
	if (game.game_state != GAMESTATE_GAMEPLAY && game.game_state != GAMESTATE_RACE_START)
       return;
   
  
   
  /* if(current_players == 0)
	{
	game.game_state = GAMESTATE_PAUSED;	
	}*/
	
	for(int p = 0; p < game.players; p++)
	{
 
		if(players[p].can_be_hurt)
		{
	
			if (KEY_PRESS(players[p].gamepad,PER_DGT_KL) || players[p].cpu_left)
						{//left
						
							if(players[p].enable_controls)
							{
								if(!players[p].cpu_left)
								{
								players[p].ry -= players[p].physics_turn_speed;	
								}else
								{
								players[p].ry -= players[p].physics_speed;		
								}		
								players[p].physics_grip = 0.8f;
							}
							
						players[p].wheel_ry =-45;
						
						}
			else
			if (KEY_PRESS(players[p].gamepad,PER_DGT_KR) || players[p].cpu_right)
					{//right
						
						if(players[p].enable_controls)
						{						
							if(!players[p].cpu_right)
							{
							players[p].ry += players[p].physics_turn_speed;
							}else
							{
							players[p].ry += players[p].physics_speed;	
							}
						
							players[p].physics_grip = -0.8f;						
						}
						
					players[p].wheel_ry =45;	
					}
			
			else
			{players[p].wheel_ry =0;}
		
			if((KEY_PRESS(players[p].gamepad,PER_DGT_KL) || players[p].cpu_left) && players[p].physics_speed > (players[p].physics_max_speed/2))
			{
			//jo_audio_stop_sound(&drift_sound);
			//jo_audio_play_sound_on_channel(&drift_sound, 3);
			if(!players[p].physics_is_in_air)
			{
			pcm_play(players[p].drift_sound, PCM_ALT_LOOP, players[p].volume);			
			players[p].drift = true;
			}	
			}else
			
			if((KEY_PRESS(players[p].gamepad,PER_DGT_KR)|| players[p].cpu_right) && players[p].physics_speed > (players[p].physics_max_speed/2))
			{
			
			if(!players[p].physics_is_in_air)
			{
			pcm_play(players[p].drift_sound, PCM_ALT_LOOP, players[p].volume);
			players[p].drift = true;
			}	
			}else
			{
			players[p].drift = false;
			pcm_cease(players[p].drift_sound);			
			}
		
		
			if ((KEY_PRESS(players[p].gamepad,PER_DGT_TB)&& players[p].enable_controls) || players[p].cpu_gas)
						{	
							if(players[p].physics_speed < 0.0f)
							{
							physics_decelerate_backwards(p);	
							}else
							{
								if(players[p].current_powerup == 1 && players[p].powerup_active)
								{
								physics_boost(p);
								}else if(players[p].current_powerup == 4 && players[p].powerup_active)
								{
								physics_accelerate_forwards_small(p);
								}else
								{
								physics_accelerate_forwards(p);
								}
							}
						// make 10 different pitch engine sounds, 0 being idle, 10 being high pitch then play different ones depending on the speed from 0 -> 10+
						// for negative speed (reverse), just play a high pitch one
						//pcm_play(eng1_sound, PCM_ALT_LOOP, 4);
						}//up
			else
			if ((KEY_PRESS(players[p].gamepad,PER_DGT_TA)&&players[p].enable_controls)|| players[p].cpu_brake)
						{
						if(players[p].physics_speed > 0.0f)
						{
						physics_decelerate_forwards(p);	
						}else
						{
						physics_accelerate_backwards(p);
						}
						
						}//down
			//else
			//{physics_apply_friction(p);
			//pcm_cease(eng1_sound);
			//}
			physics_apply_friction(p);//JJ physics change 28/01/2025
		
			//players[p].ary = players[p].physics_grip * players[p].ry;
			if(players[p].physics_grip >0)
			{
			players[p].physics_grip -= 0.2;	
			}else
			if(players[p].physics_grip <0)
			{
			players[p].physics_grip += 0.2;	
			}
			physics_apply_friction_x(p);
			physics_apply_friction_z(p);
			
			if (KEY_DOWN(players[p].gamepad,PER_DGT_TC))
			{
				stop_sounds(p);
				pcm_play(players[p].horn_sound, PCM_SEMI, 6);
				
				
				
				
			}
			if(players[p].car_selection == 4)
			{
				if (KEY_PRESS(players[p].gamepad,PER_DGT_TC) || players[p].cpu_siren)
				{
					pcm_play(players[p].siren_sound, PCM_ALT_LOOP, players[p].volume);
				}else
				{
					pcm_cease(players[p].siren_sound);	
				}
			}
			
			if (KEY_PRESS(players[p].gamepad,PER_DGT_TR))
				{
					players[p].rear_cam = true;
				}else
				{
					players[p].rear_cam = false;
				}
			 
			  
			 ///action button (use powerup)
			 // for rocket launcher, have option of 1 or 3 rockets to be used at will.
			 // for boost - add physics setting defaults as constants and change acceleration and max speed to boost versions. set timer, then change back to defaults once timer runs out.
			 if (KEY_DOWN(players[p].gamepad,PER_DGT_TL) || players[p].cpu_action)
			{	
				
				
				switch(players[p].current_powerup)
				{
				
				case 1: players[p].powerup_active = true;
						break;
						
				case 2: if(!players[p].projectile_alive)
						{
						players[p].shoot = true;
						}
						players[p].powerup_active = true;
						break;
						
				case 3:	//if(!players[p].projectile_alive)
						//{
						players[p].shoot = true;
						//}
						players[p].powerup_active = true;
						break;
						
				case 6:	//jump
						players[p].powerup_active = true;
						player_bounce(p);
						
						break;
					
				}
			}else
			{
			players[p].shoot = false;
			}
		 
		}
		
		if (KEY_DOWN(players[p].gamepad,PER_DGT_TY))
		 {	
			if(players[p].cam_zoom_num == 3)
			{
			players[p].cam_zoom_num = 0;
			}else
			{
			 players[p].cam_zoom_num ++;
			}
		players[target_player].cam_zoom_num = players[0].cam_zoom_num;
		 }
		 
		 
	
	if (players[p].ry > 180)
		players[p].ry -=360;
	else if (players[p].ry <= -180)
		players[p].ry +=360;
	
	if (players[p].cam_angle_y > 180)
		players[p].cam_angle_y -=360;
	else if (players[p].cam_angle_y <= -180)
		players[p].cam_angle_y +=360;
				
	
	
	
	//sound
	
	  //Uint8 volume = sfx_volume;
    Uint16 x_dist;
	Uint16 y_dist;
	Uint16 z_dist;
	
	//int intspeed = (int) players[0].physics_speed;
	
	if(players[p].physics_speed < 0)
	{
		stop_engine_sound(p);
		pcm_play(players[p].eng2_sound, PCM_ALT_LOOP, players[p].volume);
		
	}else if(players[p].physics_speed == 0)
	{
		stop_engine_sound(p);
		pcm_play(players[p].eng1_sound, PCM_ALT_LOOP, players[p].volume);
		
	}else if(players[p].physics_speed <1)
	{
		stop_engine_sound(p);
		pcm_play(players[p].eng1_sound, PCM_ALT_LOOP, players[p].volume);
		
	}else if(players[p].physics_speed <2)
	{
		stop_engine_sound(p);
		pcm_play(players[p].eng2_sound, PCM_ALT_LOOP, players[p].volume);
		
	}else if(players[p].physics_speed <3)
	{
		stop_engine_sound(p);
		pcm_play(players[p].eng3_sound, PCM_ALT_LOOP, players[p].volume);
		
	}else if(players[p].physics_speed <4)
	{
		
		stop_engine_sound(p);
		pcm_play(players[p].eng4_sound, PCM_ALT_LOOP, players[p].volume);
		
	}else if(players[p].physics_speed <5)
	{
		stop_engine_sound(p);
		pcm_play(players[p].eng5_sound, PCM_ALT_LOOP, players[p].volume);
		
	}else if(players[p].physics_speed <=6)
	{
		stop_engine_sound(p);
		if(players[p].physics_is_in_air)
		{
		pcm_play(players[p].eng7_sound, PCM_ALT_LOOP, players[p].volume);	
		}else
		{
		pcm_play(players[p].eng6_sound, PCM_ALT_LOOP, players[p].volume);
		}
	}else if(players[p].physics_speed >6)
	{
		stop_engine_sound(p);
		pcm_play(players[p].eng7_sound, PCM_ALT_LOOP, players[p].volume);
	}
	
		
	//dynamic volume
	//distance to player being viewed
	if(game.mode == GAMEMODE_1PLAYERRACE || game.mode == GAMEMODE_1PLAYERSURVIVAL)
	{
		x_dist = JO_ABS(players[p].x - players[target_player].x );
		y_dist = JO_ABS(players[p].y - players[target_player].y );
		z_dist = JO_ABS(players[p].z - players[target_player].z );
		
		
		players[p].volume = sfx_volume;
		
		if((x_dist + y_dist + z_dist) < 200)
		{
			players[p].volume = sfx_volume;
			//change_player_volume(p, players[p].volume, 0);
		}else if((x_dist + y_dist + z_dist) < 400)
		{
			players[p].volume = sfx_volume - 1;
		}else if((x_dist + y_dist + z_dist) < 600)
		{
			players[p].volume = sfx_volume - 2;
		}else
		{
			players[p].volume = 0;
		}
	}	
		
		
	}//player loop
	
	
	
	
		if (KEY_DOWN(0,PER_DGT_ST) && game.game_state != GAMESTATE_RACE_START)
	 {
		 if(game.pressed_start == false)
			{
				game.pause_start_time = getSeconds();
				pcm_play(pup_sound, PCM_SEMI, 6);
				if (is_cd_playing)
				{
					 CDDAStop();
					is_cd_playing = false;
				}
				
				for(int p = 0; p < game.players; p++)
				{
				stop_sounds(p);	
				}
				game.game_state = GAMESTATE_PAUSED;
				game.pause_menu = 0;
				
			}
			game.pressed_start = true;
		}
    else
		{
			game.pressed_start = false;
		}
		
		
		if (KEY_DOWN(0,PER_DGT_TX) && show_debug)
   {
	   if(target_player >= game.players-1)
	   {
	   target_player = 0;
	   }else
	   {
		target_player ++;   
	   }
		
	
   }
		
   
   /*if (KEY_DOWN(0,PER_DGT_TZ))
   {
	  
	 cam_mode ^= true;
   }*/
   
   if (KEY_PRESS(0,PER_DGT_TL) && KEY_DOWN(0,PER_DGT_TR))
	{
		ztClearText();	
	  show_debug ^= true;
	}
	
	
}


void            load_background()
{
    jo_img      bg;

    bg.data = JO_NULL;
    jo_tga_loader(&bg, "BG", "SKY.TGA", JO_COLOR_Transparent);
    jo_set_background_sprite(&bg, 0, 0);
    jo_free_img(&bg);
}


void                            draw_3d_planes(void)
{
	
	
	if (game.game_state == GAMESTATE_UNINITIALIZED)//game.game_state != GAMESTATE_GAMEPLAY && game.game_state != GAMESTATE_MAP_BUILDER && game.game_state != GAMESTATE_TITLE_SCREEN && game.game_state != GAMESTATE_END_LEVEL
       return;
	int floor_y = 0;
	int floor_rx = 0;
	int floor_ry = 0;
	int floor_rz = 0;

	if(game.game_state == GAMESTATE_END_LEVEL || game.game_state == GAMESTATE_PLAYER_SELECT || game.game_state == GAMESTATE_EXTRA_SELECT || game.game_state == GAMESTATE_OBJECT_VIEWER)
	{
		floor_x ++;
		floor_z ++;
		floor_y = -200;
		floor_rx = 0;
		floor_ry = 0;
		floor_rz = 90;
		
	}else
	{
	floor_x = JO_DIV_BY_8(players[target_player].cam_pos_x);
	floor_z = JO_DIV_BY_8(players[target_player].cam_pos_z);
	floor_rx = players[target_player].cam_angle_x;
	floor_ry = players[target_player].cam_angle_y;
	floor_rz = players[target_player].cam_angle_z;
	
	//if(cam_pos_y <=0)
	//{
	floor_y = -100 + JO_DIV_BY_8(players[target_player].cam_pos_y);
	//}
	
	}
	
	
   // SKY
   jo_sprite_set_palette(sky_pal.id);
   
    jo_3d_push_matrix();
	{
		jo_3d_rotate_matrix(floor_rz, -floor_ry, floor_rx);
		jo_3d_rotate_matrix_rad(rot.rx, rot.ry, rot.rz);
		
		jo_3d_translate_matrixf(pos.x, pos.y, pos.z);
        jo_background_3d_plane_a_draw(true);
	}
	jo_3d_pop_matrix();
	pos.y += 0.1f;
	pos.x += 0.1f;
	
	//FLOOR
	jo_sprite_set_palette(floor_pal.id);
	
	 jo_3d_push_matrix();
	{
		jo_3d_rotate_matrix(floor_rz, -floor_ry, floor_rx);
		jo_3d_rotate_matrix_rad(rot.rx, rot.ry, rot.rz);
		
		
		jo_3d_translate_matrixf(-floor_x, -floor_z, floor_y);
        jo_background_3d_plane_b_draw(true);
	}
	jo_3d_pop_matrix();
	
	jo_sprite_set_palette(font_pal.id);

}


void                init_3d_planes(void)
{
    jo_img_8bits    img;

    ///Added by XL2 : turns off the TV while we load data
    slTVOff();    

    jo_enable_background_3d_plane(JO_COLOR_Black);
	jo_set_tga_default_palette(&sky_pal);
	// SKY
    img.data = JO_NULL;
	
	jo_tga_8bits_loader(&img, "BG", level_data[game.level].sky, 0);
	    
    jo_background_3d_plane_a_img(&img, sky_pal.id, true, true);
    jo_free_img(&img);

    //FLOOR
    img.data = JO_NULL;
	
	jo_set_tga_default_palette(&floor_pal);
	
	jo_tga_8bits_loader(&img, "BG", level_data[game.level].floor, 0);
	
    jo_background_3d_plane_b_img(&img, floor_pal.id, true, false);
    jo_free_img(&img);

   ///Added by XL2 : turns on the TV
	slTVOn();
    
}

void            load_hud(void)
{
	game.hud_sprite_id = jo_sprite_add_tga_tileset("TEX", "HUD.TGA",JO_COLOR_Red,HUD_Tileset,23);
}


void			    load_4bits_car_textures(void)
{
    jo_img_8bits    img4;
    jo_texture_definition   *ftexture;
	int id;
	
	for(int tex_num = 0; tex_num < 12; tex_num++)
	{	
	img_4bpp *current_texture=(img_4bpp *)car_img_data[tex_num];
	
	img4.data = NULL;
    img4.data = current_texture->palette_id;
    img4.width = JO_DIV_BY_2(current_texture->width);  //Since the image is 4 bits per pixel, we divide the width by 2 to fit each 2 pixels in 1 Byte
    img4.height = current_texture->height;
    id = jo_sprite_add_8bits_image(&img4);  //Adds the image in memory
    ftexture=&__jo_sprite_def[id];
    ftexture->width=current_texture->width;  //Ghetto technique for compatibility with Jo Engine, but trying to replace the sprite will throw an error
    __jo_sprite_pic[id].color_mode=COL_16; 
    ftexture->size = JO_MULT_BY_32(current_texture->width & 0x1f8) | ftexture->height;

    set_palette(current_texture->palette, id);
	}
}



void			    load_4bits_player_textures(void)
{
    jo_img_8bits    img4;
    jo_texture_definition   *ftexture;
	int id;
	
	for(int tex_num = 0; tex_num < 33; tex_num++)
	{	
	img_4bpp *current_texture=(img_4bpp *)player1_img_data[tex_num];
	
	img4.data = NULL;
    img4.data = current_texture->palette_id;
    img4.width = JO_DIV_BY_2(current_texture->width);  //Since the image is 4 bits per pixel, we divide the width by 2 to fit each 2 pixels in 1 Byte
    img4.height = current_texture->height;
    id = jo_sprite_add_8bits_image(&img4);  //Adds the image in memory
    ftexture=&__jo_sprite_def[id];
    ftexture->width=current_texture->width;  //Ghetto technique for compatibility with Jo Engine, but trying to replace the sprite will throw an error
    __jo_sprite_pic[id].color_mode=COL_16; 
    ftexture->size = JO_MULT_BY_32(current_texture->width & 0x1f8) | ftexture->height;

    set_palette(current_texture->palette, id);
	}
	
	
	
	for(int tex_num = 0; tex_num < 33; tex_num++)
	{	
	img_4bpp *current_texture=(img_4bpp *)player2_img_data[tex_num];
	
	img4.data = NULL;
    img4.data = current_texture->palette_id;
    img4.width = JO_DIV_BY_2(current_texture->width);  //Since the image is 4 bits per pixel, we divide the width by 2 to fit each 2 pixels in 1 Byte
    img4.height = current_texture->height;
    id = jo_sprite_add_8bits_image(&img4);  //Adds the image in memory
    ftexture=&__jo_sprite_def[id];
    ftexture->width=current_texture->width;  //Ghetto technique for compatibility with Jo Engine, but trying to replace the sprite will throw an error
    __jo_sprite_pic[id].color_mode=COL_16; 
    ftexture->size = JO_MULT_BY_32(current_texture->width & 0x1f8) | ftexture->height;

    set_palette(current_texture->palette, id);
	}
	
	for(int tex_num = 0; tex_num < 33; tex_num++)
	{	
	img_4bpp *current_texture=(img_4bpp *)player3_img_data[tex_num];
	
	img4.data = NULL;
    img4.data = current_texture->palette_id;
    img4.width = JO_DIV_BY_2(current_texture->width);  //Since the image is 4 bits per pixel, we divide the width by 2 to fit each 2 pixels in 1 Byte
    img4.height = current_texture->height;
    id = jo_sprite_add_8bits_image(&img4);  //Adds the image in memory
    ftexture=&__jo_sprite_def[id];
    ftexture->width=current_texture->width;  //Ghetto technique for compatibility with Jo Engine, but trying to replace the sprite will throw an error
    __jo_sprite_pic[id].color_mode=COL_16; 
    ftexture->size = JO_MULT_BY_32(current_texture->width & 0x1f8) | ftexture->height;

    set_palette(current_texture->palette, id);
	}
	
	for(int tex_num = 0; tex_num < 33; tex_num++)
	{	
	img_4bpp *current_texture=(img_4bpp *)player4_img_data[tex_num];
	
	img4.data = NULL;
    img4.data = current_texture->palette_id;
    img4.width = JO_DIV_BY_2(current_texture->width);  //Since the image is 4 bits per pixel, we divide the width by 2 to fit each 2 pixels in 1 Byte
    img4.height = current_texture->height;
    id = jo_sprite_add_8bits_image(&img4);  //Adds the image in memory
    ftexture=&__jo_sprite_def[id];
    ftexture->width=current_texture->width;  //Ghetto technique for compatibility with Jo Engine, but trying to replace the sprite will throw an error
    __jo_sprite_pic[id].color_mode=COL_16; 
    ftexture->size = JO_MULT_BY_32(current_texture->width & 0x1f8) | ftexture->height;

    set_palette(current_texture->palette, id);
	}
}

void				total_table(void)
{
	
	int table_row = 6;
	int shadow_row = -68;
			
		for(int p = 0; p < game.players; p++)
		{
		players[p].x = 60;
		players[p].y = -52;
		players[p].z = 0;
		}
		
		jo_nbg2_printf(3,3,"          NAME      TOTAL POINTS   ");
		jo_nbg2_printf(3,4,"          ----      ------------   ");
		for(int p = 0; p < game.players; p++)
		{
			players[p].mins = players[p].best_time / 60;
			players[p].secs = players[p].best_time % 60;
			
		//table row start
		jo_sprite_change_sprite_scale_xy(74,16);
		jo_sprite_draw3D(PLAYER_TILESET + 27,148, shadow_row, 1000);shadow_row += 24;
		jo_sprite_restore_sprite_scale();
				
		players[p].z = -62 * players[p].total_position;
		//draw_player(p);
		
		table_row = 3 + (players[p].total_position *3);
		
		jo_nbg2_printf(3,table_row,"%d         PLAYER %d           %d",players[p].total_position, p+1 , players[p].total_points);
		//table row end
		
		}
	
	
}

void				race_table(void)
{
	
	int table_row = 6;
	int shadow_row = -68;
	int mins;
	int secs;
	int target_mins;
	int target_secs;
	int target_time;
	
			
		for(int p = 0; p < game.players; p++)
		{
		players[p].x = 60;
		players[p].y = -52;
		players[p].z = 0;
		}
		
		
		for(int p = 0; p < game.players; p++)
		{
			players[p].mins = players[p].best_time / 60;
			players[p].secs = players[p].best_time % 60;
			
		//table row start
		jo_sprite_change_sprite_scale_xy(74,16);
		jo_sprite_draw3D(PLAYER_TILESET + 27,148, shadow_row, 1000);shadow_row += 24;
		jo_sprite_restore_sprite_scale();
				
		players[p].z = -62 * players[p].position;
		//draw_player(p);
		
		table_row = 3 + (players[p].position *3);
		
		if(game.mode == GAMEMODE_1PLAYERRACE)
		{
		jo_nbg2_printf(3,3,"          NAME      FST LAP  POINTS");
		jo_nbg2_printf(3,4,"          ----      -------  ------");		
		jo_nbg2_printf(3,table_row,"%d         PLAYER %d    %2d.%2d      %d",players[p].position, p+1 , players[p].mins, players[p].secs, players[p].points);
		}
		
		if(game.mode == GAMEMODE_TIMEATTACK)
		{
		target_time = level_data[game.level].level_target_time;
		target_mins = target_time / 60;
		target_secs = target_time % 60;
		
		mins = players[p].total_time / 60;
		secs = players[p].total_time % 60;
			
		jo_nbg2_printf(3,3,"          TARGET    TIME     FST LAP");
		jo_nbg2_printf(3,4,"          ------    -------  -------");		
		jo_nbg2_printf(3,table_row,"         %2d.%2d     %2d.%2d    %2d.%2d",target_mins, target_secs, mins, secs, players[0].mins, players[0].secs);
		}
		
		//table row end
		
		}
	
	
}

void				end_level(void)
{
if (game.game_state != GAMESTATE_END_LEVEL)
       return;
	
	if(cd_track >0)
	{
	if (!is_cd_playing)
        {
             CDDAPlaySingle(9, true);
            is_cd_playing = true;
        }
	}
	
	int next_level;
	

	slCurWindow(winFar);
	
	move_camera(&cam1,target_player);
	
	
	
	slPushMatrix();
    {
			slRotX(DEGtoANG(players[target_player].cam_angle_z));
            slRotZ(DEGtoANG(players[target_player].cam_angle_x));
            slRotY(DEGtoANG(-players[target_player].cam_angle_y));
						
            slTranslate(toFIXED(-players[target_player].cam_pos_x-250), toFIXED(-players[target_player].cam_pos_y), toFIXED(-players[target_player].cam_pos_z+174));
			
			slTranslate(toFIXED(0), toFIXED(0), toFIXED(0));
			
			slPushMatrix();
			{
			if(use_light) computeLight();
			}
			slPopMatrix();
			
			for(int p = 0; p < game.players; p++)
			{
			draw_player(p);
			}		
	 }
	slPopMatrix();
		
	
	if(game.mode == GAMEMODE_TIMEATTACK)
	{
	if(players[0].current_time < level_data[game.level].level_target_time)
	{
	//you win, continue to next level
	if(game.level < LEVEL_MENU_MAX -1)
	{
	next_level = game.level +1;
	}else
	{
	next_level = 1;	
	}
	jo_nbg2_printf(14, 10, "YOU DID IT!");
	race_table();
	
	//menu
	game.end_level_menu = menu((char**)menu_endlevelwin,2,14,22,game.end_level_menu);

	}else
	{
	//you lose, retry or quit
	next_level = game.level;
	jo_nbg2_printf(14, 10, "YOU LOSE");
	race_table();
	
	//menu
	game.end_level_menu = menu((char**)menu_endlevellose,2,14,22,game.end_level_menu);

	}
	}
	
	if(game.mode == GAMEMODE_1PLAYERSURVIVAL)
	{
	if(players[0].current_time == 0)
	{
	//you win, continue to next level
	if(game.level < LEVEL_MENU_MAX -1)
	{
	next_level = game.level +1;
	}else
	{
	next_level = 1;	
	}
	jo_nbg2_printf(14, 8, "YOU DID IT!");
	
	//menu
	game.end_level_menu = menu((char**)menu_endlevelwin,2,14,22,game.end_level_menu);

	}else
	{
	//you lose, retry or quit
	next_level = game.level;
	jo_nbg2_printf(14, 8, "YOU LOSE");
	
	//menu
	game.end_level_menu = menu((char**)menu_endlevellose,2,14,22,game.end_level_menu);

	}
	}
	
	
		if(game.mode == GAMEMODE_2PLAYERVS)
	{
		
	//which player wins, continue to next level
	if(game.level < LEVEL_MENU_MAX -1)
	{
	next_level = game.level +1;
	}else
	{
	next_level = 1;	
	}
	
	jo_nbg2_printf(14, 8 , "PLAYER %d WINS!",game.winner + 1);
	
	//menu
	game.end_level_menu = menu((char**)menu_endlevelwin,2,14,22,game.end_level_menu);
	
	}
	
	
	if(game.mode == GAMEMODE_1PLAYERRACE)
	{
		
		if(game.level < LEVEL_MENU_MAX -1)
		{
		next_level = game.level +1;
		}else
		{
		next_level = 1;	
		}
		
		//toggle tables
		if(game.show_total_table)
		{
		total_table();
		}else
		{
		race_table();
		}
				
		//menu
		game.end_level_menu = menu((char**)menu_endlevel,2,14,22,game.end_level_menu);
		
		
	}
	

	
	if (KEY_DOWN(0,PER_DGT_ST))
	 {	
		ztClearText();
		game.pressed_start = true;
		//players[0].cam_zoom_num = 1;
		
		if(players[0].best_time < level_data[game.level].level_fastest_lap || level_data[game.level].level_fastest_lap == 0)
		{
			level_data[game.level].level_fastest_lap = players[0].best_time;
		}
		
		if (game.end_level_menu == 1)
            {
				slCurWindow(winNear);
                game.game_state = GAMESTATE_UNINITIALIZED;
				clear_level();
				cam_mode = saved_cam_mode;
				
			 }
			 
            else
			{
			
				if(!game.show_total_table && game.mode == GAMEMODE_1PLAYERRACE)
				{			
				game.show_total_table = 1;
				}else
				{
				jo_sprite_free_from(game.map_sprite_id);

				ztClearText();
				jo_disable_background_3d_plane(JO_COLOR_Black);
				jo_clear_background(JO_COLOR_Black);

				game.level = next_level;
				load_level();
				load_preview(level_data[game.level].level_preview);
				load_trackmap(level_data[game.level].level_map);
				init_3d_planes();
				reset_demo();
				ztClearText();
				game.game_state = GAMESTATE_RACE_START;	
				cam_mode = saved_cam_mode;
				}
			
			}
		
	 }else
	 {
	game.pressed_start = false;	 
	 }
	
	if(rotate_cam > 360)
	{
	rotate_cam = 0;	
	}else
	{
	rotate_cam ++;	
	}
	
	
	
}

void            extra_select(void)
{
	if (game.game_state != GAMESTATE_EXTRA_SELECT)
    return;
	
	
	jo_nbg2_printf(16, 8, "****EXTRA***");

    //menu
	game.extra_menu = menu((char**)menu_extra,EXTRA_MENU_MAX,14,22,game.extra_menu);
	     
	if (KEY_DOWN(0,PER_DGT_ST))
	{	
		pcm_play(pup_sound, PCM_SEMI, 6);
		game.game_state = GAMESTATE_UNINITIALIZED;
		ztClearText();
		game.pressed_start = true;
		
		switch(game.extra_menu)
		{
		case 0:		ztClearText();
					load_level();
					game.game_state = GAMESTATE_OBJECT_VIEWER;	
					break;
		
		case 1:		ztClearText();
					load_binary((char*)"CARS.BIN", (void*)WORK_RAM_LOW);
					game.game_state = GAMESTATE_OBJECT_VIEWER;
					break;
		
		case 2:		ztClearText();
					game.game_state = GAMESTATE_LEVEL_SELECT;
					break;
				
				
			
		}	
		
		
	}
   

	
}


void			    player_select(void)
{
	if (game.game_state != GAMESTATE_PLAYER_SELECT)
		
       return;
   
	
	jo_nbg2_printf(0, 1, "              SELECT YOUR CAR");
	
   
  
    object_scale = 2.0f;
	
		
   slCurWindow(winFar);
	
	move_camera(&cam1,0);
	
	
	
	slPushMatrix();
    {
			slRotX(DEGtoANG(players[0].cam_angle_z));
            slRotZ(DEGtoANG(players[0].cam_angle_x));
            slRotY(DEGtoANG(-players[0].cam_angle_y));
            slTranslate(toFIXED(-players[0].cam_pos_x), toFIXED(-players[0].cam_pos_y-30), toFIXED(-players[0].cam_pos_z));
			
			slPushMatrix();
			{
			if(use_light) computeLight();
			}
			slPopMatrix();


			slPushMatrix();
			{
				slTranslate(toFIXED(players[0].x), toFIXED(players[0].y), toFIXED(players[0].z));
				slRotX(DEGtoANG(players[0].rx)); slRotY(DEGtoANG(players[0].ry)); slRotZ(DEGtoANG(players[0].rz));
				jo_3d_set_scalef(object_scale,object_scale,object_scale);
				{	
					slPutPolygonX((XPDATA *)car_data[0], light);
				}
				
				//wheel front_right
					slPushMatrix();
					{
					slTranslate(toFIXED(-9.8), toFIXED(1.9), toFIXED(-13.3));
					slRotY(DEGtoANG(players[0].wheel_ry));slRotX(DEGtoANG(players[0].wheel_rx)); 
										
					slPutPolygonX((XPDATA *)&xpdata_WHEEL_FR, light);
					}
					slPopMatrix();
					
					//wheel front_left
					slPushMatrix();
					{
					slTranslate(toFIXED(9.8), toFIXED(1.9), toFIXED(-13.3));
					slRotY(DEGtoANG(180));
					slRotY(DEGtoANG(players[0].wheel_ry));slRotX(DEGtoANG(-players[0].wheel_rx));
															
					slPutPolygonX((XPDATA *)&xpdata_WHEEL_FL, light);
					}
					slPopMatrix();
					
					//wheel rear_right
					slPushMatrix();
					{
					slTranslate(toFIXED(-9.8), toFIXED(1.9), toFIXED(8.7));
					
					slRotX(DEGtoANG(players[0].wheel_rx));			
					slPutPolygonX((XPDATA *)&xpdata_WHEEL_RR, light);
					}
					slPopMatrix();
					
					//wheel rear_left
					slPushMatrix();
					{
					slTranslate(toFIXED(9.8), toFIXED(1.9), toFIXED(8.7));
					slRotY(DEGtoANG(180));
					
					slRotX(DEGtoANG(-players[0].wheel_rx));				
					slPutPolygonX((XPDATA *)&xpdata_WHEEL_RL, light);
					}
					slPopMatrix();
			}
			slPopMatrix();

}
slPopMatrix();
		
		jo_sprite_draw3D(MAP_TILESET+14,-48, 74, 1000);
		jo_sprite_draw3D(MAP_TILESET+14,0, 74, 1000);
		jo_sprite_draw3D(MAP_TILESET+14,48, 74, 1000);
		
		render_CLUT_sprite(PLAYER_TILESET + 25,45,68,200);
		render_CLUT_sprite(PLAYER_TILESET + 25,45,69,200);
		render_CLUT_sprite(PLAYER_TILESET + 25,45,70,200);
		render_CLUT_sprite(PLAYER_TILESET + 25,45,71,200);
		render_CLUT_sprite(PLAYER_TILESET + 25,45,72,200);
		render_CLUT_sprite(PLAYER_TILESET + 25,45,73,200);
		render_CLUT_sprite(PLAYER_TILESET + 25,45,74,200);
		render_CLUT_sprite(PLAYER_TILESET + 25,45,75,200);
		
		render_CLUT_sprite(PLAYER_TILESET + 26,45,77,200);
		render_CLUT_sprite(PLAYER_TILESET + 26,45,78,200);
		render_CLUT_sprite(PLAYER_TILESET + 26,45,79,200);
		render_CLUT_sprite(PLAYER_TILESET + 26,45,80,200);
		render_CLUT_sprite(PLAYER_TILESET + 26,45,81,200);
		render_CLUT_sprite(PLAYER_TILESET + 26,45,82,200);
		render_CLUT_sprite(PLAYER_TILESET + 26,45,83,200);
		render_CLUT_sprite(PLAYER_TILESET + 26,45,84,200);
		

slCurWindow(winNear);

	if(game.mode == GAMEMODE_2PLAYERVS)
	{
	move_camera(&cam2,1);
		
	slPushMatrix();
    {
		
		slRotX(DEGtoANG(players[1].cam_angle_z));
        slRotZ(DEGtoANG(players[1].cam_angle_x));
        slRotY(DEGtoANG(-players[1].cam_angle_y));
        slTranslate(toFIXED(-players[1].cam_pos_x), toFIXED(-players[1].cam_pos_y-30), toFIXED(-players[1].cam_pos_z));
		
		slPushMatrix();
		{
        if(use_light) computeLight();
		}
		slPopMatrix();
	
	slPushMatrix();
			{
				slTranslate(toFIXED(players[1].x), toFIXED(players[1].y), toFIXED(players[1].z));
				slRotX(DEGtoANG(players[1].rx)); slRotY(DEGtoANG(players[1].ry)); slRotZ(DEGtoANG(players[1].rz));
				jo_3d_set_scalef(object_scale,object_scale,object_scale);
				{	
					//slPutPolygonX(xpdata_[players[1].car_selection], light);
					slPutPolygonX((XPDATA *)car_data[1], light);
				}
				
				//wheel front_right
					slPushMatrix();
					{
					slTranslate(toFIXED(-9.8), toFIXED(1.9), toFIXED(-13.3));
					slRotY(DEGtoANG(players[1].wheel_ry));slRotX(DEGtoANG(players[1].wheel_rx)); 
										
					slPutPolygonX((XPDATA *)&xpdata_WHEEL_FR, light);
					}
					slPopMatrix();
					
					//wheel front_left
					slPushMatrix();
					{
					slTranslate(toFIXED(9.8), toFIXED(1.9), toFIXED(-13.3));
					slRotY(DEGtoANG(180));
					slRotY(DEGtoANG(players[1].wheel_ry));slRotX(DEGtoANG(-players[1].wheel_rx));
															
					slPutPolygonX((XPDATA *)&xpdata_WHEEL_FL, light);
					}
					slPopMatrix();
					
					//wheel rear_right
					slPushMatrix();
					{
					slTranslate(toFIXED(-9.8), toFIXED(1.9), toFIXED(8.7));
					
					slRotX(DEGtoANG(players[1].wheel_rx));			
					slPutPolygonX((XPDATA *)&xpdata_WHEEL_RR, light);
					}
					slPopMatrix();
					
					//wheel rear_left
					slPushMatrix();
					{
					slTranslate(toFIXED(9.8), toFIXED(1.9), toFIXED(8.7));
					slRotY(DEGtoANG(180));
					
					slRotX(DEGtoANG(-players[1].wheel_rx));				
					slPutPolygonX((XPDATA *)&xpdata_WHEEL_RL, light);
					}
					slPopMatrix();
			}
			slPopMatrix();
		
	}
	slPopMatrix();
   	
		jo_sprite_draw3D(MAP_TILESET+14,-48, 74, 1000);
		jo_sprite_draw3D(MAP_TILESET+14,0, 74, 1000);
		jo_sprite_draw3D(MAP_TILESET+14,48, 74, 1000);
		
		render_CLUT_sprite(PLAYER2_TILESET + 25,45,68,200);
		render_CLUT_sprite(PLAYER2_TILESET + 25,45,69,200);
		render_CLUT_sprite(PLAYER2_TILESET + 25,45,70,200);
		render_CLUT_sprite(PLAYER2_TILESET + 25,45,71,200);
		render_CLUT_sprite(PLAYER2_TILESET + 25,45,72,200);
		render_CLUT_sprite(PLAYER2_TILESET + 25,45,73,200);
		render_CLUT_sprite(PLAYER2_TILESET + 25,45,74,200);
		render_CLUT_sprite(PLAYER2_TILESET + 25,45,75,200);
		
		render_CLUT_sprite(PLAYER2_TILESET + 26,45,77,200);
		render_CLUT_sprite(PLAYER2_TILESET + 26,45,78,200);
		render_CLUT_sprite(PLAYER2_TILESET + 26,45,79,200);
		render_CLUT_sprite(PLAYER2_TILESET + 26,45,80,200);
		render_CLUT_sprite(PLAYER2_TILESET + 26,45,81,200);
		render_CLUT_sprite(PLAYER2_TILESET + 26,45,82,200);
		render_CLUT_sprite(PLAYER2_TILESET + 26,45,83,200);
		render_CLUT_sprite(PLAYER2_TILESET + 26,45,84,200);
		
		jo_nbg2_printf(5, 22, "              ");
		jo_nbg2_printf(5, 22, car_name[players[0].car_selection]);
		jo_nbg2_printf(5, 23, "COLOUR 1");
		jo_nbg2_printf(5, 24, "COLOUR 2");
		jo_nbg2_printf(5, 25, "   OK   ");	
		jo_nbg2_printf(4, 22 + players[0].player_select_menu, "<");
		jo_nbg2_printf(17, 22 + players[0].player_select_menu, ">");
				
		jo_nbg2_printf(25, 22, "              ");
		jo_nbg2_printf(25, 22, car_name[players[1].car_selection]);
		jo_nbg2_printf(25, 23, "COLOUR 1");
		jo_nbg2_printf(25, 24, "COLOUR 2");
		jo_nbg2_printf(25, 25, "   OK   ");	
		jo_nbg2_printf(24, 22 + players[1].player_select_menu, "<");
		jo_nbg2_printf(37, 22 + players[1].player_select_menu, ">");
   
	}else
	{
		jo_nbg2_printf(14, 22, "              ");
		jo_nbg2_printf(14, 22, car_name[players[0].car_selection]);
	
		jo_nbg2_printf(14, 23, "COLOUR 1");
		jo_nbg2_printf(14, 24, "COLOUR 2");
		jo_nbg2_printf(14, 25, "     OK     ");
			
		jo_nbg2_printf(13, 22 + players[0].player_select_menu, "<");	
		jo_nbg2_printf(26, 22 + players[0].player_select_menu, ">");
		
	}
	
	if (!jo_is_pad1_available())
		return;
	for(int p = 0; p < game.players; p++)
{

if(!players[p].car_selected)
{	
players[p].ry ++;

	if (KEY_DOWN(players[p].gamepad,PER_DGT_KU))
		
    {		
            if (players[p].player_select_menu == 0)
                players[p].player_select_menu = PLAYER_SELECT_MENU_MAX;
            else
                players[p].player_select_menu --;

			ztClearText();

   }
 
   if (KEY_DOWN(players[p].gamepad,PER_DGT_KD))
    {
      
                    

            if (players[p].player_select_menu == PLAYER_SELECT_MENU_MAX)
                players[p].player_select_menu = 0;
            else
                players[p].player_select_menu ++;

            ztClearText();
     
    }

	if (KEY_DOWN(players[p].gamepad,PER_DGT_KL))
		{
			
			switch(players[p].player_select_menu)
			{
				
			case 0: 	pcm_play(cpoint_sound, PCM_SEMI, 6);   
			
						if (players[p].car_selection == 0)
						{
						players[p].car_selection = model_total-1;
							
						}else
						{
						--players[p].car_selection;
						}
						load_car(p,players[p].car_selection);
						players[p].colour1 = default_colour[players[p].car_selection][p*3];
						players[p].colour2 = default_colour[players[p].car_selection][(p*3)+1];
						players[p].colour3 = default_colour[players[p].car_selection][(p*3)+2];
						
						change_player_palette(p, 1, JO_COLOR_RGB(colour_options[players[p].colour1][0],colour_options[players[p].colour1][1],colour_options[players[p].colour1][2]));
						change_player_palette(p, 2, JO_COLOR_RGB(colour_options[players[p].colour2][0],colour_options[players[p].colour2][1],colour_options[players[p].colour2][2]));
						change_player_palette(p, 13, JO_COLOR_RGB(shadow_colour[players[p].colour3][0],shadow_colour[players[p].colour3][1],shadow_colour[players[p].colour3][2]));
						break;
						
			case 1:		if(players[p].colour1 <=0)
						{
						players[p].colour1 = MAX_COLOUR;	
						players[p].colour3 = MAX_COLOUR;
						}else
						{
						players[p].colour1 --;	
						players[p].colour3 --;	
						}
						
						change_player_palette(p, 1, JO_COLOR_RGB(colour_options[players[p].colour1][0],colour_options[players[p].colour1][1],colour_options[players[p].colour1][2]));
						change_player_palette(p, 2, JO_COLOR_RGB(colour_options[players[p].colour2][0],colour_options[players[p].colour2][1],colour_options[players[p].colour2][2]));
						change_player_palette(p, 13, JO_COLOR_RGB(shadow_colour[players[p].colour3][0],shadow_colour[players[p].colour3][1],shadow_colour[players[p].colour3][2]));
						break;
			
			
			case 2:		if(players[p].colour2 <=0)
						{
						players[p].colour2 = MAX_COLOUR;	
						}else
						{
						players[p].colour2 --;	
						}
						change_player_palette(p, 1, JO_COLOR_RGB(colour_options[players[p].colour1][0],colour_options[players[p].colour1][1],colour_options[players[p].colour1][2]));
						change_player_palette(p, 2, JO_COLOR_RGB(colour_options[players[p].colour2][0],colour_options[players[p].colour2][1],colour_options[players[p].colour2][2]));
						
						break;
			
			case 3:
						break;
						
			}
				
		}
			
					
	if (KEY_DOWN(players[p].gamepad,PER_DGT_KR))
		{
			
			switch(players[p].player_select_menu)
			{
				
			case 0:		pcm_play(cpoint_sound, PCM_SEMI, 6);   
						++players[p].car_selection;
						if (players[p].car_selection >= model_total)
						{
						players[p].car_selection = 0;	
						}  		
						load_car(p,players[p].car_selection);
						players[p].colour1 = default_colour[players[p].car_selection][p*3];
						players[p].colour2 = default_colour[players[p].car_selection][(p*3)+1];
						players[p].colour3 = default_colour[players[p].car_selection][(p*3)+2];						
						
						change_player_palette(p, 1, JO_COLOR_RGB(colour_options[players[p].colour1][0],colour_options[players[p].colour1][1],colour_options[players[p].colour1][2]));
						change_player_palette(p, 2, JO_COLOR_RGB(colour_options[players[p].colour2][0],colour_options[players[p].colour2][1],colour_options[players[p].colour2][2]));
						change_player_palette(p, 13, JO_COLOR_RGB(shadow_colour[players[p].colour3][0],shadow_colour[players[p].colour3][1],shadow_colour[players[p].colour3][2]));
						break;
						
			case 1:		if(players[p].colour1 >=MAX_COLOUR)
						{
						players[p].colour1 = 0;	
						players[p].colour3 = 0;	
						}else
						{
						players[p].colour1 ++;	
						players[p].colour3 ++;	
						}
						
						change_player_palette(p, 1, JO_COLOR_RGB(colour_options[players[p].colour1][0],colour_options[players[p].colour1][1],colour_options[players[p].colour1][2]));
						change_player_palette(p, 2, JO_COLOR_RGB(colour_options[players[p].colour2][0],colour_options[players[p].colour2][1],colour_options[players[p].colour2][2]));
						change_player_palette(p, 13, JO_COLOR_RGB(shadow_colour[players[p].colour3][0],shadow_colour[players[p].colour3][1],shadow_colour[players[p].colour3][2]));
						break;
			
			
			case 2:		if(players[p].colour2 >=MAX_COLOUR)
						{
						players[p].colour2 = 0;	
						}else
						{
						players[p].colour2 ++;	
						}
						change_player_palette(p, 1, JO_COLOR_RGB(colour_options[players[p].colour1][0],colour_options[players[p].colour1][1],colour_options[players[p].colour1][2]));
						change_player_palette(p, 2, JO_COLOR_RGB(colour_options[players[p].colour2][0],colour_options[players[p].colour2][1],colour_options[players[p].colour2][2]));
						break;
			
			case 3:
						break;
						
						
			}
			
		}
		
	if (KEY_DOWN(players[p].gamepad,PER_DGT_TA) && players[p].player_select_menu == 3)
	 {
		 players[p].car_selected = true;
		 pcm_play(pup_sound, PCM_SEMI, 6);
	 }
}else
{
players[p].ry = 0;	
}
	if (KEY_DOWN(players[p].gamepad,PER_DGT_TB))
	 {
		if(players[p].car_selected)
		{
		pcm_play(players[p].horn_sound, PCM_SEMI, 6);
		players[p].car_selected = false;
		}else
		{
		//back to title screen
		transition_to_title_screen();
		}
	 }
	 
	//show press start prompt if all players cars have been selected
	
	//1 player
	if((game.mode == GAMEMODE_1PLAYERRACE || game.mode == GAMEMODE_1PLAYERSURVIVAL) && players[0].car_selected)
		{
		jo_nbg2_printf(14, 12, "PRESS START");
				
		}else if(players[0].car_selected && players[game.players-1].car_selected)
		{
		jo_nbg2_printf(5, 12, "PRESS START");
		jo_nbg2_printf(25, 12, "PRESS START");
		
		}else
		{
		jo_nbg2_printf(14, 12, "           ");
		jo_nbg2_printf(5, 12, "           ");
		jo_nbg2_printf(25, 12, "           ");
		
		}
	 
	if (KEY_DOWN(players[p].gamepad,PER_DGT_ST))
	 {	
		
		if((game.mode == GAMEMODE_1PLAYERRACE || game.mode == GAMEMODE_1PLAYERSURVIVAL) && players[0].car_selected)
		{
			for(int op = 1; op < game.players; op++)
			{
			players[op].car_selected = true;
			players[op].car_selection = jo_random(model_total-1);
			load_car(op,players[op].car_selection);
			players[op].colour1 = default_colour[players[op].car_selection][op*3];
			players[op].colour2 = default_colour[players[op].car_selection][(op*3)+1];
			players[op].colour3 = default_colour[players[op].car_selection][(op*3)+2];
			
			change_player_palette(op, 1, JO_COLOR_RGB(colour_options[players[op].colour1][0],colour_options[players[op].colour1][1],colour_options[players[op].colour1][2]));
			change_player_palette(op, 2, JO_COLOR_RGB(colour_options[players[op].colour2][0],colour_options[players[op].colour2][1],colour_options[players[op].colour2][2]));
			change_player_palette(op, 13, JO_COLOR_RGB(shadow_colour[players[op].colour3][0],shadow_colour[players[op].colour3][1],shadow_colour[players[op].colour3][2]));
			
					
			}
		}
		
		if(players[0].car_selected && players[game.players-1].car_selected)
		{
		load_binary((char*)"TITLE.BIN", (void*)WORK_RAM_LOW);	
		players[0].car_selected = false;
		players[1].car_selected = false;
		init_1p_display();
		if(game.mode == GAMEMODE_1PLAYERSURVIVAL)
		{
		load_preview(survival_preview[0]);
		jo_set_tga_default_palette(&preview_pal);   
		preview_tex = jo_sprite_add_tga("BG", "SWB.TGA", JO_COLOR_Transparent);
		
		game.select_level = 50;
		}else
		{		
		jo_set_tga_default_palette(&preview_pal);   
		preview_tex = jo_sprite_add_tga("BG", "SBT.TGA", JO_COLOR_Transparent);
		
		jo_set_tga_default_palette(&trackmap_pal);   
		trackmap_tex = jo_sprite_add_tga("BG", "MBT1.TGA", JO_COLOR_Transparent);
		
		game.select_level = 1;
		}
		object_scale = 1.0f;
		game.game_state = GAMESTATE_LEVEL_SELECT;
		}
		
		
	 }
	 
		
		
}


		
}

void            level_select(void)
{
	if (game.game_state != GAMESTATE_LEVEL_SELECT)
    return;

	if (!is_cd_playing)
        {
             
			 CDDAPlaySingle(2, true);
            is_cd_playing = true;
        }
		
	int menu_min;
	int menu_max;
	int mins;
	int secs;
	int current_time;
	
	current_time = level_data[game.select_level].level_fastest_lap;
	
	mins = current_time / 60;
	secs = current_time % 60;
	
	if(game.mode == GAMEMODE_1PLAYERSURVIVAL)
	{
	menu_max = SURVIVAL_MENU_MAX;	
	menu_min = 50;	
	}else
	{
	menu_max = LEVEL_MENU_MAX;
	menu_min = 1;
	}
		
	move_camera(&cam1,0);
	slPushMatrix();
    {
        if(use_light) computeLight();
		
    }
	slPopMatrix();
	
	//level preview
	jo_sprite_set_palette(preview_pal.id);
	jo_sprite_draw3D(preview_tex,0, 0, 300);
	//jo_sprite_set_palette(font_pal.id);
	
	//level map
	jo_sprite_set_palette(trackmap_pal.id);
	jo_sprite_change_sprite_scale(2);
	jo_sprite_draw3D(trackmap_tex,76, 76, 75);
	jo_sprite_restore_sprite_scale();
	jo_sprite_set_palette(font_pal.id);

	//logo
	jo_sprite_draw3D(MAP_TILESET+2,-72, -98, 100);
	jo_sprite_draw3D(MAP_TILESET+3,-24, -98, 100);
	jo_sprite_draw3D(MAP_TILESET+4,24, -98, 100);
	jo_sprite_draw3D(MAP_TILESET+5,72, -98, 100);
	
	jo_sprite_draw3D(MAP_TILESET+6,-72, -50, 100);
	jo_sprite_draw3D(MAP_TILESET+7,-24, -50, 100);
	jo_sprite_draw3D(MAP_TILESET+8,24, -50, 100);
	jo_sprite_draw3D(MAP_TILESET+9,72, -50, 100);
	
	jo_sprite_draw3D(MAP_TILESET+14,-72, 74, 100);
	jo_sprite_draw3D(MAP_TILESET+14,-24, 74, 100);
	jo_sprite_draw3D(MAP_TILESET+14,24, 74, 100);
	jo_sprite_draw3D(MAP_TILESET+14,72, 74, 100);
	
	//level name
	jo_nbg2_printf(10, 22, "             ");
	jo_nbg2_printf(10, 22, level_data[game.select_level].level_name);
	jo_nbg2_printf(10, 24, level_data[game.select_level].difficulty);
	if(game.mode == GAMEMODE_TIMEATTACK)
	{
	jo_nbg2_printf(10, 26, "TARGET TIME %02d.%02d", level_data[game.select_level].level_target_time / 60,level_data[game.select_level].level_target_time % 60);
	}else
	{
	jo_nbg2_printf(10, 26,  "BEST LAP %02d.%02d", mins,secs);
		
	//jo_nbg2_printf(10, 26, "BEST LAP 00.26");	
	}
	jo_nbg2_printf(9, 22, "<");	
	jo_nbg2_printf(26, 22, ">");
	//

	
 slPushMatrix();
    {
			slRotX(DEGtoANG(players[0].cam_angle_z));
            slRotZ(DEGtoANG(players[0].cam_angle_x));
            slRotY(DEGtoANG(-players[0].cam_angle_y));
            slTranslate(toFIXED(-players[0].cam_pos_x), toFIXED(-players[0].cam_pos_y), toFIXED(-players[0].cam_pos_z));

	
	slPushMatrix();
				{
					slTranslate(toFIXED(map_section[0].x), toFIXED(map_section[0].y), toFIXED(map_section[0].z+100));
					slRotY(DEGtoANG(players[0].effect_ry));
										
					slPutPolygonX(map_section[0].map_model, light);
				}
				slPopMatrix();
				
	}
				slPopMatrix();
				
	players[0].effect_ry ++;
	
	if (players[0].effect_ry > 180)
		players[0].effect_ry -=360;
	else if (players[0].effect_ry <= -180)
		players[0].effect_ry +=360;
	

   
	
    if (KEY_DOWN(0,PER_DGT_KL))
    {
		pcm_play(cpoint_sound, PCM_SEMI, 6);   
		ztClearText();
        if (game.select_level == menu_min)
        {game.select_level = menu_max;}
        else
        {game.select_level --;}
	
			if(game.mode == GAMEMODE_1PLAYERSURVIVAL)
			{
			load_preview(survival_preview[game.select_level-menu_min]);	
			}else
			{
			load_preview(level_data[game.select_level].level_preview);
			load_trackmap(level_data[game.select_level].level_map);
			}			
    }
       

    if (KEY_DOWN(0,PER_DGT_KR))
    {
		pcm_play(cpoint_sound, PCM_SEMI, 6);   
		ztClearText();
        if (game.select_level == menu_max)
        {game.select_level = menu_min;}
        else
        {game.select_level ++;}
		
			if(game.mode == GAMEMODE_1PLAYERSURVIVAL)
			{
			load_preview(survival_preview[game.select_level-menu_min]);	
			}else
			{
			load_preview(level_data[game.select_level].level_preview);
			load_trackmap(level_data[game.select_level].level_map);
			}	
    }
	
	
	if (KEY_DOWN(0,PER_DGT_TR))
    {
        ztClearText();
		game.game_state = GAMESTATE_EXTRA_SELECT;
    }
	
	game.level = game.select_level;
	
	
	if (KEY_DOWN(0,PER_DGT_TB))
    {
	//back to title screen
	transition_to_title_screen();
	}
    // did player one pause the game?
   if (KEY_DOWN(0,PER_DGT_ST))
    {
        if(game.pressed_start == false)
        {
			
			load_level();
			if(game.mode == GAMEMODE_2PLAYERVS)
			{
			init_2p_display();	
			}
			init_3d_planes();
			reset_demo();
			ztClearText();
			game.game_state = GAMESTATE_RACE_START;	
        }
        game.pressed_start = true;
    }
    else
    {
        game.pressed_start = false;
    }

	
}

void gameLoop(void)
{
   	while (1)
    {
		sdrv_vblank_rq();
        slUnitMatrix(0);
		draw_3d_planes();
        my_gamepad();
		cpu_control();
		race_start();
        my_draw();
		pause_game();
		object_viewer();
		title_screen();
		player_select();
		level_select();
		extra_select();
		end_level();
		slSynch();
    }
}

void			load_player_and_enemies(void)
{	
	load_4bits_car_textures();
	load_4bits_player_textures();
	jo_sprite_add_tga_tileset("TEX", "PUP.TGA",JO_COLOR_Green,PUP_Tileset,8);		
}


void			load_sound(void)
{
	//23040, 15360, 11520, or 7680Hz can be used.
	players[0].eng1_sound = load_16bit_pcm((Sint8 *)"ENG1.PCM", 15360);
	players[0].eng2_sound = load_16bit_pcm((Sint8 *)"ENG2.PCM", 15360);
	players[0].eng3_sound = load_16bit_pcm((Sint8 *)"ENG3.PCM", 15360);
	players[0].eng4_sound = load_16bit_pcm((Sint8 *)"ENG4.PCM", 15360);
	players[0].eng5_sound = load_16bit_pcm((Sint8 *)"ENG5.PCM", 15360);
	players[0].eng6_sound = load_16bit_pcm((Sint8 *)"ENG6.PCM", 15360);
	players[0].eng7_sound = load_16bit_pcm((Sint8 *)"ENG7.PCM", 15360);
	players[0].horn_sound = load_16bit_pcm((Sint8 *)"HORN.PCM", 15360);
	players[0].drift_sound = load_16bit_pcm((Sint8 *)"DRIFT.PCM", 15360);
	players[0].siren_sound = load_16bit_pcm((Sint8 *)"COP.PCM", 11520);
	
	players[1].eng1_sound = load_16bit_pcm((Sint8 *)"ENG1.PCM", 15360);
	players[1].eng2_sound = load_16bit_pcm((Sint8 *)"ENG2.PCM", 15360);
	players[1].eng3_sound = load_16bit_pcm((Sint8 *)"ENG3.PCM", 15360);
	players[1].eng4_sound = load_16bit_pcm((Sint8 *)"ENG4.PCM", 15360);
	players[1].eng5_sound = load_16bit_pcm((Sint8 *)"ENG5.PCM", 15360);
	players[1].eng6_sound = load_16bit_pcm((Sint8 *)"ENG6.PCM", 15360);
	players[1].eng7_sound = load_16bit_pcm((Sint8 *)"ENG7.PCM", 15360);
	players[1].horn_sound = load_16bit_pcm((Sint8 *)"HORN.PCM", 15360);
	players[1].drift_sound = load_16bit_pcm((Sint8 *)"DRIFT.PCM", 15360);
	players[1].siren_sound = load_16bit_pcm((Sint8 *)"COP.PCM", 15360);
	
	players[2].eng1_sound = load_16bit_pcm((Sint8 *)"ENG1.PCM", 15360);
	players[2].eng2_sound = load_16bit_pcm((Sint8 *)"ENG2.PCM", 15360);
	players[2].eng3_sound = load_16bit_pcm((Sint8 *)"ENG3.PCM", 15360);
	players[2].eng4_sound = load_16bit_pcm((Sint8 *)"ENG4.PCM", 15360);
	players[2].eng5_sound = load_16bit_pcm((Sint8 *)"ENG5.PCM", 15360);
	players[2].eng6_sound = load_16bit_pcm((Sint8 *)"ENG6.PCM", 15360);
	players[2].eng7_sound = load_16bit_pcm((Sint8 *)"ENG7.PCM", 15360);
	players[2].horn_sound = load_16bit_pcm((Sint8 *)"HORN.PCM", 15360);
	players[2].drift_sound = load_16bit_pcm((Sint8 *)"DRIFT.PCM", 15360);
	players[2].siren_sound = load_16bit_pcm((Sint8 *)"COP.PCM", 15360);
	
	players[3].eng1_sound = load_16bit_pcm((Sint8 *)"ENG1.PCM", 15360);
	players[3].eng2_sound = load_16bit_pcm((Sint8 *)"ENG2.PCM", 15360);
	players[3].eng3_sound = load_16bit_pcm((Sint8 *)"ENG3.PCM", 15360);
	players[3].eng4_sound = load_16bit_pcm((Sint8 *)"ENG4.PCM", 15360);
	players[3].eng5_sound = load_16bit_pcm((Sint8 *)"ENG5.PCM", 15360);
	players[3].eng6_sound = load_16bit_pcm((Sint8 *)"ENG6.PCM", 15360);
	players[3].eng7_sound = load_16bit_pcm((Sint8 *)"ENG7.PCM", 15360);
	players[3].horn_sound = load_16bit_pcm((Sint8 *)"HORN.PCM", 15360);
	players[3].drift_sound = load_16bit_pcm((Sint8 *)"DRIFT.PCM", 15360);
	players[3].siren_sound = load_16bit_pcm((Sint8 *)"COP.PCM", 15360);
	
	 crash_sound = load_16bit_pcm((Sint8 *)"CRASH.PCM", 15360);
	pup_sound = load_16bit_pcm((Sint8 *)"PUP.PCM", 15360);
	 cpoint_sound = load_16bit_pcm((Sint8 *)"CPOINT.PCM", 15360);
	 boing_sound = load_16bit_pcm((Sint8 *)"BOING.PCM", 15360);
	//effect_sound = load_16bit_pcm((Sint8 *)"EFFECT.PCM", 15360);
	 explosion_sound = load_16bit_pcm((Sint8 *)"EXPL.PCM", 15360);
	

}

void                    load_nbg2_font(void)
{
	//jo_set_tga_palette_handling(JO_NULL);
    jo_img_8bits        img;
	img.data = NULL;
	jo_set_tga_default_palette(&font_pal);
    jo_tga_8bits_loader(&img, JO_ROOT_DIR, "FONT.TGA", 2);
	
	
	//font_pal.data[0] = JO_COLOR_RGB(0, 0, 0); // You can change values at any time (see line 44)
   // font_pal.data[1] = JO_COLOR_RGB(255, 127, 39);
	
    jo_vdp2_set_nbg2_8bits_font(&img, " 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ!\"?=%&',.()*+-/<>", font_pal.id, false, true);
    jo_free_img(&img);
}


void			jo_main(void)
{
	init_display();
	init_1p_display();
	load_drv(ADX_MASTER_2304);
	
	 /**Added by XL2 **/
	slDynamicFrame(ON); //Dynamic framerate, when the VDP1 can't fully draw to the framebuffer in the allocated amount of time (1/60, 2/60, etc.) it will continue drawing when it's ON. Else it will try to complete drawing by skipping lines and details and finish in the allocated time. If the app runs well with the value at OFF, leave it at OFF!
    SynchConst=(Sint8)2;  //Framerate control. 1/60 = 60 FPS, 2/60 = 30 FPS, etc.
	framerate=2;
	
	jo_create_palette(&floor_pal);
	jo_create_palette(&sky_pal);
	jo_create_palette(&preview_pal);
	jo_create_palette(&trackmap_pal);
	jo_create_palette(&font_pal);
	load_nbg2_font();
	
	jo_nbg2_printf(10, 22, "LOADING");
	
	load_player_and_enemies();
	jo_nbg2_printf(10, 22, "LOADING.");
	load_hud();
	jo_nbg2_printf(10, 22, "LOADING..");
	load_sound();
	jo_nbg2_printf(10, 22, "LOADING...");	
	
	game.map_sprite_id = jo_sprite_add_tga_tileset("TEX", "TITLE.TGA",JO_COLOR_Red,MAP_Tileset,44);
	
	jo_nbg2_printf(10, 22, "LOADING....");
	
	load_binary((char*)"TITLE.BIN", (void*)WORK_RAM_LOW);
	
	jo_nbg2_printf(10, 22, "LOADING.....");

	init_3d_planes();
	jo_nbg2_printf(10, 22, "LOADING......");
		
	pos.x = 800.0;
	pos.y = 800.0;
	pos.z = 35.0;

	rot.rx = JO_DEG_TO_RAD(90.0);
	rot.ry = JO_DEG_TO_RAD(0.0);
	rot.rz = JO_DEG_TO_RAD(0.0);	
	
	game.players = 1;
	CDDASetVolume(music_vol);
	jo_nbg2_printf(10, 22, "             ");
	game.game_state = GAMESTATE_TITLE_SCREEN;
	slZdspLevel(5);
	gameLoop();
}

/*
** END OF FILE
*/
