#ifndef VIDEO_HW_H
#define VIDEO_HW_H


typedef struct
{
    // Screen width at every line
    //
    int     screen_width[256];

    // Background (effective scroll positions at every line) for VDC 1/2
    //
    int     x_scroll[256];
    int     y_scroll[256];

    int     prev_vblank;
    int     cur_vblank;

    int     prev_bg;
    int     cur_bg;

    int     prev_spr;
    int     cur_spr;

    int     prev_hdw;
    int     cur_hdw;

    int     prev_hds;
    int     cur_hds;

    int     prev_hsw;
    int     cur_hsw;

    int     start_render_line;
    bool    force_flush;
    bool    skip;

} vdc_hw_struct;


extern vdc_hw_struct vdc_hw_a;
extern vdc_hw_struct vdc_hw_b;

#ifdef EXTERN_C_START
EXTERN_C_START
#endif 

void update_palette_frame(int pal);
void reset_video_hw();
void render_line_hw(void);
void render_line_sgx_hw(void);
void render_line_force_flush(void);

#ifdef EXTERN_C_END
EXTERN_C_END
#endif 

#endif