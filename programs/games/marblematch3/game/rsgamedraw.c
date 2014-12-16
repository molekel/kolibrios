#include "rsgamedraw.h"
#include "rsgametext.h"
#include "rsgamemenu.h"

#include "rsgentex.h"
#include "rs/rsplatform.h"

#include "rskos.h"

#include "rsnoise.h"

#include "strings.h"


void game_draw() {

	int w = GAME_WIDTH;
	int h = GAME_HEIGHT;
	
	if (game.need_redraw) {
    
        

        if ( (game.status == STATUS_MENU) || (game.status == STATUS_LOADING) ) {
                
            texture_draw(&game.framebuffer, &game.tex_bg, 0, 0, DRAW_MODE_REPLACE);
            
            if (game.menu_index == MENU_MAIN) {
                    
                if (game.status == STATUS_LOADING) {
                    game_textout( GAME_WIDTH/2 - 192, 240, 0, "        L0ADING```      " );
                }
                else {

                    texture_draw( &game.framebuffer, &game.tex_logo, 0, 50, DRAW_MODE_ALPHA );

                    if (game.time) {
                        
                        game_textout( GAME_WIDTH/2 - 192, 230, 3, "     LEVEL PA55ED     " );
                        
                        char s[] = "     TIME: 000         ";
                        int time_sec = game.time / 25;
                        s[11] = '0' + (( time_sec / 100 ) % 10);
                        s[12] = '0' + (( time_sec / 10 ) % 10);
                        s[13] = '0' + (( time_sec / 1 ) % 10);
                        game_textout( GAME_WIDTH/2 - 192, 260, 3, s );
                    };

                    game_textout( GAME_WIDTH/2 - 192, 300, 0, "     CLICK T0 5TART     " );
                    
                    
                };
                
                game_textout( 2, GAME_HEIGHT-10, 2,  "DEVEL0PED BY R0MAN 5HUVAL0V");
            };
        
        }
        else {
                
            texture_draw(&game.framebuffer, &game.tex_bg_gameplay, 0, 0, DRAW_MODE_REPLACE);
            
            int i, j;
            for (i = 0; i < FIELD_HEIGHT; i++) {
                for (j = 0; j < FIELD_WIDTH; j++) {
                    if ( IS_BIT_SET( game.field[i*FIELD_WIDTH + j], CRYSTAL_VISIBLE_BIT )) {
                        texture_draw( &game.framebuffer, &game.tex_crystals[ game.field[i*FIELD_WIDTH + j] & CRYSTAL_INDEX_MASK ], FIELD_X0+ j*CRYSTAL_SIZE, FIELD_Y0+ i*CRYSTAL_SIZE, DRAW_MODE_ALPHA );
                        if (game.selected) {
                            if ( (j == game.selected_x) && (i == game.selected_y) ) {
                                texture_draw( &game.framebuffer, &game.tex_cursor, FIELD_X0+ j*CRYSTAL_SIZE, FIELD_Y0+ i*CRYSTAL_SIZE, DRAW_MODE_ALPHA );
                            };
                        };
                    };
                };
            };
            
            for (i = 0; i < game.explosions_count; i++) {
                texture_draw( &game.framebuffer, &(game.tex_explosion[  (game.explosions[i]>>16) & 0xFF  ]), 
                    FIELD_X0 + CRYSTAL_SIZE*( game.explosions[i] & 0xFF) - (EXPLOSION_SIZE-CRYSTAL_SIZE)/2 , 
                    FIELD_Y0 + CRYSTAL_SIZE*( (game.explosions[i]>>8) & 0xFF) - (EXPLOSION_SIZE-CRYSTAL_SIZE)/2 , 
                    DRAW_MODE_ALPHA);
            };


            char str[] = "TIME: 999    ";
            int time_sec = game.time / 25;
            str[6] = '0' + ( (time_sec / 100) % 10);
            str[7] = '0' + ( (time_sec / 10) % 10);
            str[8] = '0' + ( (time_sec / 1) % 10);
            
            game_textout( 56, 32, 3, str );
            
            char sstr[] = "5C0RE: 000 0F 100   ";
            sstr[7] = '0' + ( (game.score / 100) % 10);
            sstr[8] = '0' + ( (game.score / 10) % 10);
            sstr[9] = '0' + ( (game.score / 1) % 10);
            
            game_textout( 56, 64, 3, sstr );
   
        };


        rskos_draw_area(0, 0, w, h, game.window_scale, game.framebuffer.data, game.scaled_framebuffer);
	};
	
	game.need_redraw = 0;

};



void game_textures_init_stage1() {
    
    int i;
    
    texture_init(&game.framebuffer, GAME_WIDTH, GAME_HEIGHT);
    
//    texture_init(&game.tex, 64, 64);
//    rs_gen_init(1, 64);
//    rs_gen_func_set(0, 0.0);
//    rs_gen_func_cell(0, 1200, 10, NULL, 1.0, 1.0, 1.0, 1.0, 0.0, 1.0);
//    rs_gen_func_normalize(0, 0.0, 1.0);
//    rs_gen_func_posterize(0, 5);
//    rs_gen_tex_out_rgba(0, 0, 0, -1, 1.0, 1.0, 1.0, 1.0);
//    memcpy(game.tex.data, rs_gen_reg.tex_out, 64*64*4 );
//    rs_gen_term();
    
    texture_init(&game.tex_clouds, 128, 128);
    rs_gen_init(1, 128);
    rs_gen_func_perlin(0, 8, 5, 0.5, 1100);
    rs_gen_func_normalize(0, 0.0, 1.0);
    rs_gen_func_posterize(0, 6);
    rs_gen_func_mult_add_value(0, 0, 0.6, 0.4);
//    rs_gen_func_set(0, 1.0);
    rs_gen_tex_out_rgba(0, 0, 0, -1, 1.0, 1.0, 1.0, 1.0);
    memcpy(game.tex_clouds.data, rs_gen_reg.tex_out, 128*128*4 );
    rs_gen_term();


    

    texture_init(&game.tex_logo, GAME_WIDTH, 128);
    texture_clear(&game.tex_logo, COLOR_TRANSPARENT);
    
    game_textout_adv( &game.tex_logo, GAME_WIDTH/2 - 192, 3, 1, DRAW_MODE_REPLACE, "MARBLE");
    game_textout_adv( &game.tex_logo, GAME_WIDTH/2 - 192, 63, 1, DRAW_MODE_REPLACE, "MATCH3");
    texture_draw(&game.tex_logo, &game.tex_clouds, 0, 0, DRAW_MODE_MULT | DRAW_TILED_FLAG);
    game_textout_adv( &game.tex_logo, GAME_WIDTH/2 - 192 - 4, 0, 1, DRAW_MODE_MULT, "MARBLE");
    game_textout_adv( &game.tex_logo, GAME_WIDTH/2 - 192 - 4, 60, 1, DRAW_MODE_MULT, "MATCH3");
    

    
    texture_init(&game.tex_bg, 512, 512);
    texture_clear(&game.tex_bg, COLOR_SILVER);

    texture_init(&game.tex_bg_gameplay, 512, 512);
    texture_clear(&game.tex_bg_gameplay, COLOR_SILVER);
    
    texture_init(&game.tex_cursor, CRYSTAL_SIZE, CRYSTAL_SIZE);
    rs_gen_init(2, CRYSTAL_SIZE);
    
    rs_gen_func_set(0, 0.0); // inner
    rs_gen_func_radial(0, 0.5, 0.5, 0.5, 1.0, 2.0);
    rs_gen_func_clamp(0, 0.1, 0.5);
    rs_gen_func_normalize(0, 0.0, 1.0);
    rs_gen_func_mult_add_value(0, 0, -1.0, 1.0);

    rs_gen_func_set(1, 0.0); // outer
    rs_gen_func_radial(1, 0.5, 0.5, 0.5, 1.0, 2.0);
    rs_gen_func_clamp(1, 0.0, 0.2);
    rs_gen_func_normalize(1, 0.0, 1.0);
    
    rs_gen_func_mult(0, 0, 1);

    rs_gen_func_set(1, 1.0);
//    rs_gen_tex_out_rgba_set(0.0, 0.0, 0.0, 0.0);
    rs_gen_tex_out_rgba(1, 1, 1, 0, 1.0, 1.0, 1.0, 1.0);
    memcpy(game.tex_cursor.data, rs_gen_reg.tex_out, CRYSTAL_SIZE*CRYSTAL_SIZE*4 );
    rs_gen_term();

    texture_init(&game.tex_field, FIELD_WIDTH*CRYSTAL_SIZE + 7, FIELD_HEIGHT*CRYSTAL_SIZE + 7);
    texture_clear(&game.tex_field, 0xAABBBBBB); // 0x66404060 // 0xAACCCCCC

    
//    float cr_r[CRYSTALS_COUNT] = { 0.8, 0.2, 0.1, 0.6, 0.7, 0.0, 0.7 };
//    float cr_g[CRYSTALS_COUNT] = { 0.1, 0.6, 0.4, 0.0, 0.6, 0.0, 0.8 };
//    float cr_b[CRYSTALS_COUNT] = { 0.1, 0.1, 0.7, 0.7, 0.0, 0.3, 0.9 };

//    float cr_r[CRYSTALS_COUNT] = { 0.9, 0.3, 0.1, 0.7, 0.8, 0.0, 0.8 };
//    float cr_g[CRYSTALS_COUNT] = { 0.1, 0.8, 0.5, 0.0, 0.7, 0.0, 0.8 };
//    float cr_b[CRYSTALS_COUNT] = { 0.0, 0.1, 0.9, 0.8, 0.0, 0.5, 0.9 };
    
    float cr_r[CRYSTALS_COUNT] = { 1.0, 0.4, 0.1, 0.9, 0.9, 0.2, 0.8 };
    float cr_g[CRYSTALS_COUNT] = { 0.1, 1.0, 0.6, 0.1, 0.8, 0.2, 0.8 };
    float cr_b[CRYSTALS_COUNT] = { 0.0, 0.1, 1.0, 1.0, 0.0, 0.9, 0.9 };
    

    rs_gen_init(5, CRYSTAL_SIZE);
    for (i = 0; i < CRYSTALS_COUNT; i++) {
    
        texture_init(&(game.tex_crystals[i]), CRYSTAL_SIZE, CRYSTAL_SIZE);
        
        rs_gen_func_set(0, 0.0);
        rs_gen_func_radial(0, 0.5, 0.5, 0.5, 0.75, 10.0);

//        rs_gen_func_perlin(2, 33, 4, 0.5, 350+i);
//        rs_gen_func_normalize(2, 0.0, 1.0);
//        rs_gen_func_posterize(2, 4);
//        
//        rs_gen_func_cell(1, 410+i, 50, NULL, -2.0, 1.0, 1.0, 1.0, 0.0, 1.0);
//        rs_gen_func_posterize(1, 2);
//        rs_gen_func_normalize(1, 0.0, 1.0);
//        rs_gen_func_add(1, 1, 2, 1.0, 0.5);
//        rs_gen_func_normalize(1, 0.0, 1.0);
//        rs_gen_func_posterize(1, 4);
//
//        rs_gen_func_add(1, 0, 1, 1.0, 1.0);
//        rs_gen_func_normalize(1, 0.0, 1.0);
//        rs_gen_func_mult(1, 0, 1);
//        rs_gen_func_normalize(1, 0.0, 1.0);
//        rs_gen_func_posterize(1, 4);
        
        rs_gen_func_set(1, 0.0);
        rs_gen_func_cell(1, 110+100*i, 7+i, NULL, 1.0, 1.0, 0.0, 1.0, 0.0, 1.0);
        rs_gen_func_normalize(1, 0.0, 1.0);
//        rs_gen_func_mult_add_value(1, 1, 0.9, 0.1);
        
//        rs_gen_func_normalmap(2, 3, 3, 1, 1.0);
//        rs_gen_func_mult(1, 1, 2);
        
        //rs_gen_tex_out_rgba_set(0.0, 0.0, 0.0, 0.0);
        //rs_gen_tex_out_rgba(1, 1, 1, 1, 0.5+ 0.03*(i%2), 0.7+ 0.03*(i%3) , 0.9, 1.0);
//        rs_gen_tex_out_rgba_set(0.2 + 0.2*(i/3), 0.2 + 0.1*(i%5), 0.2 + 0.1*(i%7), 0.0);
//        rs_gen_tex_out_rgba(1, 1, 1, 1, 0.0, 0.0, 0.0, 1.0);
        
        rs_gen_tex_out_rgba_set(0.0, 0.0, 0.0, 0.0);
//        rs_gen_tex_out_rgba_set( cr_b[i], cr_g[i], cr_r[i], 0.0);
        rs_gen_tex_out_rgba(1, 1, 1, 0, cr_b[i], cr_g[i], cr_r[i], 1.0);

        memcpy(game.tex_crystals[i].data, rs_gen_reg.tex_out, CRYSTAL_SIZE*CRYSTAL_SIZE*4 );
    };
    rs_gen_term();
    
    
    
    rs_gen_init(3, EXPLOSION_SIZE);
    for (i = 0; i < EXPLOSION_FRAMES_COUNT; i++) {
            
        texture_init(&(game.tex_explosion[i]), EXPLOSION_SIZE, EXPLOSION_SIZE);

        rs_gen_func_set(0, 1.0);
//        rs_gen_func_radial(0, 0.5, 0.5, 0.3 + 0.5*i/EXPLOSION_FRAMES_COUNT, 0.975, 4.0);
//        rs_gen_func_set(0, 1.0);

        rs_gen_func_set(1, 0.0);
        rs_gen_func_radial(1, 0.5, 0.5, 0.1 + 0.4*i/EXPLOSION_FRAMES_COUNT, 1.0 - 1.0*i/EXPLOSION_FRAMES_COUNT, 2.5 + i%5);

        rs_gen_tex_out_rgba_set( 0.0, 0.0, 0.0, 0.0);
        rs_gen_tex_out_rgba(0, 0, 0, 1, 1.0, 1.0, 1.0, 1.0);

        memcpy(game.tex_explosion[i].data, rs_gen_reg.tex_out, EXPLOSION_SIZE*EXPLOSION_SIZE*4 );
    };
    rs_gen_term();
};

void game_textures_init_stage2() {
     
//            texture_clear(&game.tex_bg, COLOR_SILVER);
//             /*
        
    rs_gen_init(6, 512);
    rs_gen_func_perlin(0, 8, 5, 0.5, 1100);
    rs_gen_func_normalize(0, 0.0, 1.0);
    rs_gen_func_perlin(1, 8, 5, 0.5, 1700);
    rs_gen_func_normalize(1, 0.0, 1.0);
    rs_gen_func_cell(2, 1118, 50, NULL, 1.0, 0.887, -0.333, 1.0, 0.0, 4.0); // 1360
    rs_gen_func_normalize(2, 0.0, 0.5);

    rs_gen_func_adr(3, 2, 0, 1, 1.0, 0.3);
    
    rs_gen_func_inv(3, 3, 7.5);
    rs_gen_func_normalize(3, 0.0, 1.0);

//            signed short c[] = { 0, 250, 250, 0, 500, 250, 250, 500};
    signed short c[] = { 0, 0, 0, 512, 512, 0, 512, 512};
//            signed short c[] = { 128, 128, 128, 384, 384, 128, 384, 384};
    //rs_gen_func_cell(4, 0, 4, c, 0.0, 0.3, 1.0, 0.5, 0.0, 0.30);
    rs_gen_func_cell(4, 0, 4, c, 1.0, 0.3, 0.0, 0.95, 0.0, 0.30);
    rs_gen_func_normalize(4, 0.0, 1.0);

//            rs_gen_func_radial(5, 0.5, 0.5, 0.60, 1.0, 4.0);
//            rs_gen_func_add(4, 4, 5, 0.5, 0.5);
//
    rs_gen_func_mult(4, 4, 3);
    
    // coloring...
    rs_gen_func_mult_add_value(0, 4, 0.8, 0.0);
    rs_gen_func_add(0, 4, 1, 0.95, 0.05);
    rs_gen_func_add(3, 4, 2, 0.95, 0.05);

    
    rs_gen_tex_out_rgba(4, 0, 3, -1, 0.9, 0.9, 0.9, 1.0);
    memcpy(game.tex_bg.data, rs_gen_reg.tex_out, 512*512*4 );
    rs_gen_term();
    
    // */
    
    // Background for gameplay
    
    texture_draw( &game.tex_bg_gameplay, &game.tex_bg, 256, 256, DRAW_MODE_REPLACE | DRAW_TILED_FLAG );
    // Bevel
    texture_draw_vline( &game.tex_bg_gameplay, FIELD_X0 - 5, FIELD_Y0 - 5, FIELD_HEIGHT*CRYSTAL_SIZE + 10, 0xFF404060 );
    texture_draw_hline( &game.tex_bg_gameplay, FIELD_X0 - 5, FIELD_Y0 - 5, FIELD_WIDTH*CRYSTAL_SIZE + 10, 0xFF404060 );
    texture_draw_vline( &game.tex_bg_gameplay, FIELD_X0 - 4, FIELD_Y0 - 4, FIELD_HEIGHT*CRYSTAL_SIZE + 8, 0xFF606080 );
    texture_draw_hline( &game.tex_bg_gameplay, FIELD_X0 - 4, FIELD_Y0 - 4, FIELD_WIDTH*CRYSTAL_SIZE + 8, 0xFF606080 );
    
    texture_draw_vline( &game.tex_bg_gameplay, FIELD_X0 + 4 + FIELD_WIDTH*CRYSTAL_SIZE, FIELD_Y0 - 4, FIELD_HEIGHT*CRYSTAL_SIZE + 8, 0xFFC0C0C0 );
    texture_draw_hline( &game.tex_bg_gameplay, FIELD_X0 - 4, FIELD_Y0 + 4 + FIELD_HEIGHT*CRYSTAL_SIZE, FIELD_WIDTH*CRYSTAL_SIZE + 9, 0xFFC0C0C0 );
    texture_draw_vline( &game.tex_bg_gameplay, FIELD_X0 + 5 + FIELD_WIDTH*CRYSTAL_SIZE, FIELD_Y0 - 5, FIELD_HEIGHT*CRYSTAL_SIZE + 10, 0xFFE0E0E0 );
    texture_draw_hline( &game.tex_bg_gameplay, FIELD_X0 - 5, FIELD_Y0 + 5 + FIELD_HEIGHT*CRYSTAL_SIZE, FIELD_WIDTH*CRYSTAL_SIZE + 11, 0xFFE0E0E0 );
    
    texture_draw( &game.tex_bg_gameplay, &game.tex_field, FIELD_X0 - 3, FIELD_Y0 - 3, DRAW_MODE_ALPHA );
    
    
};

void game_textures_free() {
    free(game.scaled_framebuffer);
    
    //    texture_free(&game.tex_gui_line);
    
    int i;
    for (i = 0; i < CRYSTALS_COUNT; i++) {
        texture_free(&game.tex_crystals[i]);
    };
    
    for (i = 0; i < EXPLOSION_FRAMES_COUNT; i++) {
        texture_free(&game.tex_explosion[i]);
    };
    
    texture_free(&game.framebuffer);
    
    texture_free(&game.tex_logo);
    texture_free(&game.tex_clouds);
    
    texture_free(&game.tex_bg);
    texture_free(&game.tex_bg_gameplay);
};
