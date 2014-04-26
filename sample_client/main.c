#include <gsKit.h>
#include <dmaKit.h>
#include <malloc.h>
#include <stdio.h>
#include <math.h>
#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <gsToolkit.h>
#include <libds3ps2.h>


struct SS_BUTTONS
{
    u8 select   : 1;
    u8 L3       : 1;
    u8 R3       : 1;
    u8 start    : 1;    
    u8 up       : 1;        
    u8 right    : 1;
    u8 down     : 1;
    u8 left     : 1;


    u8 L2       : 1;
    u8 R2       : 1;
    u8 L1       : 1;
    u8 R1       : 1;
    u8 triangle : 1;    
    u8 circle   : 1;
    u8 cross    : 1;    
    u8 square   : 1;

    u8 PS       : 1;
    u8 not_used : 7;
};

struct SS_ANALOG
{
    u8 x;
    u8 y;
};

struct SS_DPAD_SENSITIVE
{
    u8 up;
    u8 right;
    u8 down;
    u8 left;
};

struct SS_SHOULDER_SENSITIVE
{
    u8 L2;
    u8 R2;
    u8 L1;
    u8 R1;
};

struct SS_BUTTON_SENSITIVE
{
    u8 triangle;
    u8 circle;
    u8 cross;
    u8 square;
};

struct SS_MOTION
{
    u16 acc_x;
    u16 acc_y;
    u16 acc_z;
    u16 z_gyro;
};

struct SS_GAMEPAD
{
    u8                        hid_data;
    u8                        unk0;
    struct SS_BUTTONS         buttons;
    u8                        unk1;
    struct SS_ANALOG          left_analog;
    struct SS_ANALOG          right_analog;
    u32                       unk2;
    struct SS_DPAD_SENSITIVE       dpad_sens;
    struct SS_SHOULDER_SENSITIVE   shoulder_sens;
    struct SS_BUTTON_SENSITIVE     button_sens;
    u16                       unk3;
    u8                        unk4;
    u8                        status;
    u8                        power_rating;
    u8                        comm_status;
    u32                       unk5;
    u32                       unk6;
    u8                        unk7;
    struct SS_MOTION               motion;
    u8 lala[40];
} __attribute__((packed, aligned(32)));


#define DEG2RAD(x) ((x)*0.01745329251)
void draw_circle(GSGLOBAL *gsGlobal, float x, float y, float radius, u64 color, u8 filled);
void print_data(struct SS_GAMEPAD *data);
void random_leds();

GSGLOBAL *gsGlobal;
GSFONTM *gsFontM;
u64 White, Black, FontColor, Red, Blue, CircleColor;
u8 r = 0xFF, g=0x00, b=0x00;

int main(void)
{
    gsGlobal = gsKit_init_global();
    gsFontM = gsKit_init_fontm();

    dmaKit_init(D_CTRL_RELE_OFF,D_CTRL_MFD_OFF, D_CTRL_STS_UNSPEC,
    D_CTRL_STD_OFF, D_CTRL_RCYC_8, 1 << DMA_CHANNEL_GIF);

    // Initialize the DMAC
    dmaKit_chan_init(DMA_CHANNEL_GIF);
    dmaKit_chan_init(DMA_CHANNEL_FROMSPR);
    dmaKit_chan_init(DMA_CHANNEL_TOSPR);

    Black = GS_SETREG_RGBAQ(0x00,0x00,0x00,0xFF,0x00);
    White = GS_SETREG_RGBAQ(0xFF,0xFF,0xFF,0x00,0x00);
    Red = GS_SETREG_RGBAQ(0xFF,0x00,0x00,0x00,0x00);
    Blue = GS_SETREG_RGBAQ(0x00,0x00,0xFF,0x00,0x00);
    FontColor = GS_SETREG_RGBAQ(0x2F,0x20,0x20,0xFF,0x00);

    gsGlobal->PrimAlphaEnable = GS_SETTING_ON;
    gsKit_init_screen(gsGlobal);
    gsKit_fontm_upload(gsGlobal, gsFontM);
    gsFontM->Spacing = 0.75f;
    gsKit_mode_switch(gsGlobal, GS_ONESHOT);
    
    SifLoadModule("mass:/ds3ps2.irx", 0, NULL);
    struct SS_GAMEPAD ds3;
    ds3ps2_init();
    random_leds();
    
    float pos_x = gsGlobal->Width/2, pos_y = gsGlobal->Height/2;

    while (1) {
        gsKit_clear(gsGlobal, White);
        
        memset(&ds3, 0x0, sizeof(struct SS_GAMEPAD));
        ds3ps2_get_input((void*)&ds3);
        //correct_data(&ds3);
        
        /*if (ds3.L1) {pos_x = gsGlobal->Width/2, pos_y = gsGlobal->Height/2;}
        if (ds3.R1) {random_leds();}
        //1920x940
        if (ds3.finger1active) {
            draw_circle(gsGlobal, (gsGlobal->Width/1920.0f)*ds3.finger1X, (gsGlobal->Height/940.0f)*ds3.finger1Y, 14, Red, 1);
        }
        if (ds3.finger2active) {
            draw_circle(gsGlobal, (gsGlobal->Width/1920.0f)*ds3.finger2X, (gsGlobal->Height/940.0f)*ds3.finger2Y, 14, Blue, 1);
        }
        */
        
        #define THRESHOLD 25.0f
        if (fabs(ds3.motion.acc_x) > THRESHOLD)
            pos_y -= ds3.motion.acc_x/55.0f;
        if (fabs(ds3.motion.acc_y) > THRESHOLD)
            pos_x -= ds3.motion.acc_y/55.0f;
        
        CircleColor = GS_SETREG_RGBAQ(r, g, b, 0x00, 0x00);
        draw_circle(gsGlobal, pos_x, pos_y, 19, CircleColor, 0);
        draw_circle(gsGlobal, pos_x, pos_y, 18, CircleColor, 0);
        draw_circle(gsGlobal, pos_x, pos_y, 17, CircleColor, 0);
        draw_circle(gsGlobal, pos_x, pos_y, 16, CircleColor, 0);
     
        print_data(&ds3);

        gsKit_sync_flip(gsGlobal);
        gsKit_queue_exec(gsGlobal);
    }

    return 0;
}

void random_leds()
{
    r = rand()%0xFF;
    g = rand()%0xFF;
    b = rand()%0xFF;
    //ds3ps2_set_led(1);
    //ds3ps2_send_ledsrumble();
}


void print_data(struct SS_GAMEPAD *data)
{ 
    char text[512];
    int y = 20, x = 5;
    
    sprintf(text,"PS: %i   START: %i   SELECT: %i   /\\: %i   []: %i   O: %i   X: %i", \
            data->buttons.PS, data->buttons.start, data->buttons.select, data->buttons.triangle, \
            data->buttons.square, data->buttons.circle, data->buttons.cross);
    gsKit_fontm_print_scaled(gsGlobal, gsFontM, x, y+=30, 3, 0.5f, FontColor, text);

    sprintf(text,"L3: %i   R3: %i   L1: %i   L2: %i   R1: %i   R2: %i", \
             data->buttons.L3, data->buttons.R3, data->buttons.L1, data->buttons.L2, data->buttons.R1, data->buttons.R2);
    gsKit_fontm_print_scaled(gsGlobal, gsFontM, x, y+=30, 3, 0.5f, FontColor, text);

    sprintf(text,"UP: %i   DOWN: %i   RIGHT: %i   LEFT: %i   LX: %i   LY: %i   RX: %i   RY: %i", \
            data->buttons.up, data->buttons.down, data->buttons.right, data->buttons.left,
            data->left_analog.x, data->left_analog.y, data->right_analog.x, data->right_analog.y);
    gsKit_fontm_print_scaled(gsGlobal, gsFontM, x, y+=30, 3, 0.5f, FontColor, text);

    sprintf(text,"aX: %i   aY: %i   aZ: %i   Zgyro: %i", \
            data->motion.acc_x, data->motion.acc_y, data->motion.acc_z, data->motion.z_gyro);
    gsKit_fontm_print_scaled(gsGlobal, gsFontM, x, y+=30, 3, 0.5f, FontColor, text);

    sprintf(text,"L1 predata: %i   L2 predata: %i   R1 predata: %i   R2 predata: %i", \
            data->shoulder_sens.L1, data->shoulder_sens.L2, data->shoulder_sens.R1, data->shoulder_sens.R2);
    gsKit_fontm_print_scaled(gsGlobal, gsFontM, x, y+=30, 3, 0.5f, FontColor, text);

    sprintf(text,"/\\ predata: %i   [] predata: %i   O predata: %i   X predata: %i",
            data->button_sens.triangle, data->button_sens.square, data->button_sens.circle, data->button_sens.cross);
    gsKit_fontm_print_scaled(gsGlobal, gsFontM, x, y+=30, 3, 0.5f, FontColor, text);

    sprintf(text,"UP: %i   DOWN: %i   RIGHT: %i   LEFT: %i", \
            data->dpad_sens.up, data->dpad_sens.down, data->dpad_sens.right, data->dpad_sens.left);
    gsKit_fontm_print_scaled(gsGlobal, gsFontM, x, y+=30, 3, 0.5f, FontColor, text);
            
}

void draw_circle(GSGLOBAL *gsGlobal, float x, float y, float radius, u64 color, u8 filled)
{    
    float v[37*2];
    int a;
    float ra;
    for (a = 0; a < 36; a++) {
        ra = DEG2RAD(a*10);
        v[a*2] = cos(ra) * radius + x;
        v[a*2+1] = sin(ra) * radius + y;
    }
    if (!filled) {
        v[72] = radius + x;
        v[73] = y;
    }
    
    if (filled) gsKit_prim_triangle_fan(gsGlobal, v, 36, 3, color);
    else        gsKit_prim_line_strip(gsGlobal, v, 37, 3, color);
}

