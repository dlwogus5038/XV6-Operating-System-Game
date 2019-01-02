#include "systetris.h"
#include "display.h"

struct tet
{
    int x;
    int y;
    int r;
    int t;
    block_t blocks[4];
} curr_tet;

int start_tetris = 0;
int score = 0;

// tetris module variables
static well_t well = 
{ 
    .x = WELL_LEFT, 
    .y = WELL_TOP, 
    .w = WELL_WIDTH*BLOCK_WIDTH, 
    .h = WELL_HEIGHT*BLOCK_HEIGHT 
};

well_bounds_t well_bounds = 
{
    .l = BOARD_LEFT,
    .r = BOARD_RIGHT,
    .t = BOARD_TOP,
    .b = BOARD_BOT
};
static block_t blocks[NUM_BLOCKS];

// static function prototypes
static void draw_block(int x, int y, int c);
static void draw_blocks();
static void draw_tet();
static void set_tet();
static void clear_rows();
static int lock_tet();
static int check_kick();
static int left_bound();
static int right_bound();
static void move_left();
static void move_right();
static int move_down();

static void set_tet_i();
static void set_tet_o();
static void set_tet_t();
static void set_tet_s();
static void set_tet_z();
static void set_tet_j();
static void set_tet_l();

/* Global functions */
int tetris_score()
{
    return score;
}

void tetris_init()
{
    int i;
    for (i = 0; i < NUM_BLOCKS; i++)
        blocks[i].p = 0;
    score = 0;
}

void tetris_new(int seed)
{
    curr_tet.t = seed%NUM_TET_TYPES;
    curr_tet.x = BOARD_LEFT+3*BLOCK_WIDTH;
    curr_tet.y = BOARD_TOP;
    curr_tet.r = 0;         //rotation

    set_tet();
}

void tetris_rotate()
{
    int r = curr_tet.r;
    curr_tet.r = (curr_tet.r+1)%4;
    set_tet();
    if (check_kick())
    {
        curr_tet.r = r;
        set_tet();
    }

    // check for collision (do kick if needed)
    //kick_tet();
}

int tetris_move(tet_move_t tet_move)
{
    switch (tet_move)
    {
        case TET_MOVE_LEFT:
            move_left();
            return 0;
        case TET_MOVE_RIGHT:
            move_right();
            return 0;
        case TET_MOVE_DOWN:
            if (move_down())
            {
                return 0;
            }
            else
            {
                if (lock_tet())
                {
                    return -1;
                }
                else
                {
                    clear_rows();
                    return 1;
                }
            }
        case TET_MOVE_DROP:
            while (move_down());
            return 0;
        default:
            return -2;
    }
}

void tetris_update()
{
    int i, j;

#if MODE_UNCHAINED
    int p;
    // clear screen to black
    for (p = 0; p < NUM_FRAMES; p++)
        for (i = 0; i < FRAME_PIX; i++)
            frame_buffer[p][i] = 0; 

    // draw well
    for (p = 0; p < NUM_FRAMES; p++) 
        for (i = well.x; i < well.x+well.w; i++)
            for (j = well.y; j < well.y+BLOCK_HEIGHT; j++)
                frame_buffer[p][((i>>2)+((j*SCREEN_WIDTH)>>2))] = 21;
    for (p = 0; p < NUM_FRAMES; p++) 
        for (i = well.x; i < well.x+well.w; i++)
            for (j = well.y+well.h-BLOCK_HEIGHT+1; j < well.y+well.h; j++)
                frame_buffer[p][((i>>2)+((j*SCREEN_WIDTH)>>2))] = 21;
    for (p = 0; p < NUM_FRAMES; p++) 
        for (i = well.x; i < well.x+BLOCK_WIDTH; i++)
            for (j = well.y; j < well.y+well.h; j++)
                frame_buffer[p][((i>>2)+((j*SCREEN_WIDTH)>>2))] = 21;
    for (p = 0; p < NUM_FRAMES; p++) 
        for (i = well.x+well.w-BLOCK_WIDTH+1; i < well.x+well.w; i++)
            for (j = well.y; j < well.y+well.h; j++)
                frame_buffer[p][((i>>2)+((j*SCREEN_WIDTH)>>2))] = 21;
#else
    // clear screen to black
    for (i = 0; i < SCREEN_PIX; i++)
        frame_buffer[i] = 0; 

    // draw well
    for (i = well.x; i < well.x+well.w; i++)
        for (j = well.y; j < well.y+BLOCK_HEIGHT; j++)
            frame_buffer[SCREEN_WIDTH*j + i] = 21;
    for (i = well.x; i < well.x+well.w; i++)
        for (j = well.y+well.h-BLOCK_HEIGHT+1; j < well.y+well.h; j++)
            frame_buffer[SCREEN_WIDTH*j + i] = 21;
    for (i = well.x; i < well.x+BLOCK_WIDTH; i++)
        for (j = well.y; j < well.y+well.h; j++)
            frame_buffer[SCREEN_WIDTH*j + i] = 21;
    for (i = well.x+well.w-BLOCK_WIDTH+1; i < well.x+well.w; i++)
        for (j = well.y; j < well.y+well.h; j++)
            frame_buffer[SCREEN_WIDTH*j + i] = 21;
#endif
    // draw the current tet
    draw_tet();

    // draw locked blocks in well
    draw_blocks();
}

/* Module functions */
void draw_blocks()
{
    int i;
    for (i = 0; i < NUM_BLOCKS; i++)
        if (blocks[i].p)
            draw_block(blocks[i].x, blocks[i].y, blocks[i].c);
}

void set_tet()
{
    switch (curr_tet.t)
    {
        case TET_TYPE_I:
            set_tet_i();
            break;
        case TET_TYPE_O:
            set_tet_o();
            break;
        case TET_TYPE_T:
            set_tet_t();
            break;
        case TET_TYPE_S:
            set_tet_s();
            break;
        case TET_TYPE_Z:
            set_tet_z();
            break;
        case TET_TYPE_J:
            set_tet_j();
            break;
        case TET_TYPE_L:
            set_tet_l();
            break;
       default:
            break;
    }
}

int check_kick()
{
    
    int i, j, dl, dr, db;

    // check if any two blocks overlap
    for (i = 0; i < NUM_BLOCKS; i++)
        for (j = 0; j < 4; j++)
            if (blocks[i].p && blocks[i].x == curr_tet.blocks[j].x && blocks[i].y == curr_tet.blocks[j].y)
                return 1;

    // check against left well wall
    for (i = 0; i < 4; i++)
    {
        dl = well_bounds.l - curr_tet.blocks[i].x;
        if (dl > 0)
            return 1;
    }

    // check against right well wall
    for (i = 0; i < 4; i++)
    {
        dr = curr_tet.blocks[i].x - well_bounds.r + 1;
        if (dr > 0)
            return 1;
    }

    // check against well bottom
    for (i = 0; i < 4; i++)
    {
        db = curr_tet.blocks[i].y - well_bounds.b;
        if (db > 0)
            return 1;
    }

    return 0;
}

void kick_tet()
{
    int dx, mdx = 0;

    int i;
    for (i = 0; i < 4; i++)
    {
        dx = well_bounds.l - curr_tet.blocks[i].x;
        if (dx > mdx)
            mdx = dx;
    }
    
    curr_tet.x += mdx;
    for (i = 0; i < 4; i++);
        curr_tet.blocks[i].x += mdx;
    /*
    int dx;
    dx = well_bounds.l - curr_tet.b.l;
    if (dx > 0)
       curr_tet.x += dx; 
    dx = well_bounds.r - curr_tet.b.r;
    if (dx < 0)
       curr_tet.x += dx;
    */
}

int left_bound()
{
    int i, x=10000;
    for (i = 0; i < 4; i++)
    {
        if (curr_tet.blocks[i].x < x)
            x = curr_tet.blocks[i].x;
    }
    return x;
}

int right_bound()
{
    int i, x=-1;
    for (i = 0; i < 4; i++)
    {
        if (curr_tet.blocks[i].x > x)
            x = curr_tet.blocks[i].x;
    }
    return x;
}

void move_left()
{
    int i, j;

    // check if any blocks to the left of this one
    for (i = 0; i < NUM_BLOCKS; i++)
        if (blocks[i].p)
            for (j = 0; j < 4; j++)
                if (curr_tet.blocks[j].y == blocks[i].y)
                    if (curr_tet.blocks[j].x-BLOCK_WIDTH == blocks[i].x)
                        return;

    // check if touching left well wall
    if (left_bound() > well_bounds.l)
    {
        curr_tet.x -= BLOCK_WIDTH;
        for (i = 0; i < 4; i++)
            curr_tet.blocks[i].x -= BLOCK_WIDTH;
    }
}

void move_right()
{
    int i, j;

    // check if any blocks to the right of this one
    for (i = 0; i < NUM_BLOCKS; i++)
        if (blocks[i].p)
            for (j = 0; j < 4; j++)
                if (curr_tet.blocks[j].y == blocks[i].y)
                    if (curr_tet.blocks[j].x+BLOCK_WIDTH == blocks[i].x)
                        return;

    // check if touching right well wall
    if (right_bound()+BLOCK_WIDTH < well_bounds.r)
    {
        curr_tet.x += BLOCK_WIDTH;
        for (i = 0; i < 4; i++)
            curr_tet.blocks[i].x += BLOCK_WIDTH;
    }
}

int move_down()
{
    int i, j;
    
    // check if curr tet is on top of any well blocks
    for (i = 0; i < NUM_BLOCKS; i++)
        if (blocks[i].p)
            for (j = 0; j < 4; j++)
                if (curr_tet.blocks[j].x == blocks[i].x)
                    if (curr_tet.blocks[j].y+BLOCK_HEIGHT == blocks[i].y)
                        return 0;

    // check if curr tet is on well floor
    for (i = 0; i < 4; i++)
        if (curr_tet.blocks[i].y == well_bounds.b)
            return 0;

    // if not touching any blocks or floor, then move down
    curr_tet.y += BLOCK_HEIGHT;
    for (i = 0; i < 4; i++)
        curr_tet.blocks[i].y += BLOCK_HEIGHT;
    return 1;
}

int lock_tet()
{
    int i, j;
    for (i = 0, j = 0; i < 4 && j < NUM_BLOCKS; j++)
    {
        if (blocks[j].p == 0)
        {
            blocks[j].x = curr_tet.blocks[i].x;
            blocks[j].y = curr_tet.blocks[i].y;
            blocks[j].c = curr_tet.blocks[i].c;
            blocks[j].p = 1;
            i++;
        }
    }

    for (i = 0; i < 4; i++)
        if (curr_tet.blocks[i].y == BOARD_TOP)
            return 1;
    return 0; 
}

void clear_rows()
{
    int i, idx;
    int rows[NUM_ROWS];

    // init rows to all 0
    for (i = 0; i < NUM_ROWS; i++)
        rows[i] = 0;

    // how many blocks are in each row
    for (i = 0; i < NUM_BLOCKS; i++)
    {
        if (blocks[i].p)
        {
            idx = blocks[i].y/BLOCK_HEIGHT - 1;
            rows[idx]++;
        }
    }

    // mark all full rows
    for (i = 0; i < NUM_ROWS; i++)
    {
        if (rows[i] == NUM_COLS)
            rows[i] = 1;
        else
           rows[i] = 0; 
    }

    // clear all full rows
    for (i = 0; i < NUM_BLOCKS; i++)
    {
        if (blocks[i].p)
        {
            idx = blocks[i].y/BLOCK_HEIGHT - 1;
            if (rows[idx])
                blocks[i].p = 0;
        }
    }

    // get idx of a cleared row
    int cr = 0;
    for (i = 0; i < NUM_ROWS; i++)
    {
        if (rows[i])
        {
            idx = i;
            cr++;
        }
    }

    // drop all blocks above cleared rows
    int j;
    if (cr > 0)
    {
        for (i = 0; i < NUM_BLOCKS; i++)
        {
            if (blocks[i].p)
            {
                j = blocks[i].y/BLOCK_HEIGHT - 1;
                if (j < idx)
                    blocks[i].y += cr*BLOCK_HEIGHT;
            }
        }
        score += 100*cr;
    }
}

void draw_block(int x, int y, int c)
{
    int i, j;
#if MODE_UNCHAINED
    int p;
    for (p = 0; p < NUM_FRAMES; p++) 
        for (i = x+1; i < x+BLOCK_WIDTH; i++)
            for (j = y+1; j < y+BLOCK_HEIGHT; j++)
                frame_buffer[p][((i>>2)+((j*SCREEN_WIDTH)>>2))] = c;
#else
    for (i = x+1; i < x+BLOCK_WIDTH; i++)
        for (j = y+1; j < y+BLOCK_HEIGHT; j++)
            frame_buffer[SCREEN_WIDTH*j+i] = c;
#endif
}

void draw_tet()
{
    int i;
    for (i = 0; i < 4; i++)
        draw_block(curr_tet.blocks[i].x, curr_tet.blocks[i].y, curr_tet.blocks[i].c);
}

void set_tet_i()
{
    int i;
    switch (curr_tet.r)
    {
        case 0:
            for (i = 0; i < 4; i++)
            {
                curr_tet.blocks[i].x = curr_tet.x + i*BLOCK_WIDTH;
                curr_tet.blocks[i].y = curr_tet.y + BLOCK_WIDTH;
                curr_tet.blocks[i].c = TET_COLOR_I;
            }
            break;
        case 1:
            for (i = 0; i < 4; i++)
            {
                curr_tet.blocks[i].x = curr_tet.x + 2*BLOCK_WIDTH;
                curr_tet.blocks[i].y = curr_tet.y + i*BLOCK_WIDTH;
                curr_tet.blocks[i].c = TET_COLOR_I;
            }
            break;
        case 2:
            for (i = 0; i < 4; i++)
            {
                curr_tet.blocks[i].x = curr_tet.x + i*BLOCK_WIDTH;
                curr_tet.blocks[i].y = curr_tet.y + 2*BLOCK_WIDTH;
                curr_tet.blocks[i].c = TET_COLOR_I;
            }
            break;
        case 3:
            for (i = 0; i < 4; i++)
            {
                curr_tet.blocks[i].x = curr_tet.x + BLOCK_WIDTH;
                curr_tet.blocks[i].y = curr_tet.y + i*BLOCK_WIDTH;
                curr_tet.blocks[i].c = TET_COLOR_I;
            }
            break;
        default:
            break;
    }
}

void set_tet_o()
{
    int i, j;
    for (i = 1; i < 3; i++)
        for (j = 0; j < 2; j++)
        {
            curr_tet.blocks[(i-1)*2 + j].x = curr_tet.x + i*BLOCK_WIDTH;
            curr_tet.blocks[(i-1)*2 + j].y = curr_tet.y + j*BLOCK_HEIGHT;
            curr_tet.blocks[(i-1)*2 + j].c = TET_COLOR_O;
        }
}

void set_tet_t()
{
    int i;
    for (i = 0; i < 4; i++)
        curr_tet.blocks[i].c = TET_COLOR_T;
    switch (curr_tet.r)
    {
        case 0:
            curr_tet.blocks[0].x = curr_tet.x + BLOCK_WIDTH;
            curr_tet.blocks[0].y = curr_tet.y + BLOCK_HEIGHT;
            curr_tet.blocks[1].x = curr_tet.x;
            curr_tet.blocks[1].y = curr_tet.y + BLOCK_HEIGHT;
            curr_tet.blocks[2].x = curr_tet.x + 2*BLOCK_WIDTH;
            curr_tet.blocks[2].y = curr_tet.y + BLOCK_HEIGHT;
            curr_tet.blocks[3].x = curr_tet.x + BLOCK_WIDTH;
            curr_tet.blocks[3].y = curr_tet.y;
            break;
        case 1:
            curr_tet.blocks[0].x = curr_tet.x + BLOCK_WIDTH;
            curr_tet.blocks[0].y = curr_tet.y + BLOCK_HEIGHT;
            curr_tet.blocks[1].x = curr_tet.x + BLOCK_WIDTH;
            curr_tet.blocks[1].y = curr_tet.y + 2*BLOCK_HEIGHT;
            curr_tet.blocks[2].x = curr_tet.x + 2*BLOCK_WIDTH;
            curr_tet.blocks[2].y = curr_tet.y + BLOCK_HEIGHT;
            curr_tet.blocks[3].x = curr_tet.x + BLOCK_WIDTH;
            curr_tet.blocks[3].y = curr_tet.y;
            break;
        case 2:
            curr_tet.blocks[0].x = curr_tet.x + BLOCK_WIDTH;
            curr_tet.blocks[0].y = curr_tet.y + BLOCK_HEIGHT;
            curr_tet.blocks[1].x = curr_tet.x;
            curr_tet.blocks[1].y = curr_tet.y + BLOCK_HEIGHT;
            curr_tet.blocks[2].x = curr_tet.x + 2*BLOCK_WIDTH;
            curr_tet.blocks[2].y = curr_tet.y + BLOCK_HEIGHT;
            curr_tet.blocks[3].x = curr_tet.x + BLOCK_WIDTH;
            curr_tet.blocks[3].y = curr_tet.y + 2*BLOCK_HEIGHT;
            break;
        case 3:
            curr_tet.blocks[0].x = curr_tet.x + BLOCK_WIDTH;
            curr_tet.blocks[0].y = curr_tet.y + BLOCK_HEIGHT;
            curr_tet.blocks[1].x = curr_tet.x;
            curr_tet.blocks[1].y = curr_tet.y + BLOCK_HEIGHT;
            curr_tet.blocks[2].x = curr_tet.x + BLOCK_WIDTH;
            curr_tet.blocks[2].y = curr_tet.y + 2*BLOCK_HEIGHT;
            curr_tet.blocks[3].x = curr_tet.x + BLOCK_WIDTH;
            curr_tet.blocks[3].y = curr_tet.y;
            break;
        default:
            break;
    }
}

void set_tet_s()
{
    int i;
    for (i = 0; i < 4; i++)
        curr_tet.blocks[i].c = TET_COLOR_S;
    switch (curr_tet.r)
    {
        case 0:
            curr_tet.blocks[0].x = curr_tet.x + BLOCK_WIDTH;
            curr_tet.blocks[0].y = curr_tet.y + BLOCK_HEIGHT;
            curr_tet.blocks[1].x = curr_tet.x;
            curr_tet.blocks[1].y = curr_tet.y + BLOCK_HEIGHT;
            curr_tet.blocks[2].x = curr_tet.x + 2*BLOCK_WIDTH;
            curr_tet.blocks[2].y = curr_tet.y;
            curr_tet.blocks[3].x = curr_tet.x + BLOCK_WIDTH;
            curr_tet.blocks[3].y = curr_tet.y;
            break;
        case 1:
            curr_tet.blocks[0].x = curr_tet.x + BLOCK_WIDTH;
            curr_tet.blocks[0].y = curr_tet.y + BLOCK_HEIGHT;
            curr_tet.blocks[1].x = curr_tet.x + BLOCK_WIDTH;
            curr_tet.blocks[1].y = curr_tet.y;
            curr_tet.blocks[2].x = curr_tet.x + 2*BLOCK_WIDTH;
            curr_tet.blocks[2].y = curr_tet.y + BLOCK_HEIGHT;
            curr_tet.blocks[3].x = curr_tet.x + 2*BLOCK_WIDTH;
            curr_tet.blocks[3].y = curr_tet.y + 2*BLOCK_HEIGHT;
            break;
        case 2:
            curr_tet.blocks[0].x = curr_tet.x + BLOCK_WIDTH;
            curr_tet.blocks[0].y = curr_tet.y + BLOCK_HEIGHT;
            curr_tet.blocks[1].x = curr_tet.x;
            curr_tet.blocks[1].y = curr_tet.y + 2*BLOCK_HEIGHT;
            curr_tet.blocks[2].x = curr_tet.x + 2*BLOCK_WIDTH;
            curr_tet.blocks[2].y = curr_tet.y + BLOCK_HEIGHT;
            curr_tet.blocks[3].x = curr_tet.x + BLOCK_WIDTH;
            curr_tet.blocks[3].y = curr_tet.y + 2*BLOCK_HEIGHT;
            break;
        case 3:
            curr_tet.blocks[0].x = curr_tet.x + BLOCK_WIDTH;
            curr_tet.blocks[0].y = curr_tet.y + BLOCK_HEIGHT;
            curr_tet.blocks[1].x = curr_tet.x;
            curr_tet.blocks[1].y = curr_tet.y;
            curr_tet.blocks[2].x = curr_tet.x;
            curr_tet.blocks[2].y = curr_tet.y + BLOCK_HEIGHT;
            curr_tet.blocks[3].x = curr_tet.x + BLOCK_WIDTH;
            curr_tet.blocks[3].y = curr_tet.y + 2*BLOCK_HEIGHT;
            break;
        default:
            break;
    }
}

void set_tet_z()
{
    int i;
    for (i = 0; i < 4; i++)
        curr_tet.blocks[i].c = TET_COLOR_Z;
    switch (curr_tet.r)
    {
            case 0:
                curr_tet.blocks[0].x = curr_tet.x + BLOCK_WIDTH;
                curr_tet.blocks[0].y = curr_tet.y + BLOCK_HEIGHT;
                curr_tet.blocks[1].x = curr_tet.x;
                curr_tet.blocks[1].y = curr_tet.y;
                curr_tet.blocks[2].x = curr_tet.x + 2*BLOCK_WIDTH;
                curr_tet.blocks[2].y = curr_tet.y + BLOCK_HEIGHT;
                curr_tet.blocks[3].x = curr_tet.x + BLOCK_WIDTH;
                curr_tet.blocks[3].y = curr_tet.y;
                break;
            case 1:
                curr_tet.blocks[0].x = curr_tet.x + BLOCK_WIDTH;
                curr_tet.blocks[0].y = curr_tet.y + BLOCK_HEIGHT;
                curr_tet.blocks[1].x = curr_tet.x + BLOCK_WIDTH;
                curr_tet.blocks[1].y = curr_tet.y + 2*BLOCK_HEIGHT;
                curr_tet.blocks[2].x = curr_tet.x + 2*BLOCK_WIDTH;
                curr_tet.blocks[2].y = curr_tet.y + BLOCK_HEIGHT;
                curr_tet.blocks[3].x = curr_tet.x + 2*BLOCK_WIDTH;
                curr_tet.blocks[3].y = curr_tet.y;
                break;
            case 2:
                curr_tet.blocks[0].x = curr_tet.x + BLOCK_WIDTH;
                curr_tet.blocks[0].y = curr_tet.y + BLOCK_HEIGHT;
                curr_tet.blocks[1].x = curr_tet.x;
                curr_tet.blocks[1].y = curr_tet.y + BLOCK_HEIGHT;
                curr_tet.blocks[2].x = curr_tet.x + BLOCK_WIDTH;
                curr_tet.blocks[2].y = curr_tet.y + 2*BLOCK_HEIGHT;
                curr_tet.blocks[3].x = curr_tet.x + 2*BLOCK_WIDTH;
                curr_tet.blocks[3].y = curr_tet.y + 2*BLOCK_HEIGHT;
                break;
            case 3:
                curr_tet.blocks[0].x = curr_tet.x + BLOCK_WIDTH;
                curr_tet.blocks[0].y = curr_tet.y + BLOCK_HEIGHT;
                curr_tet.blocks[1].x = curr_tet.x;
                curr_tet.blocks[1].y = curr_tet.y + 2*BLOCK_HEIGHT;
                curr_tet.blocks[2].x = curr_tet.x;
                curr_tet.blocks[2].y = curr_tet.y + BLOCK_HEIGHT;
                curr_tet.blocks[3].x = curr_tet.x + BLOCK_WIDTH;
                curr_tet.blocks[3].y = curr_tet.y;
                break;
            default:
                break;
        }
}

void set_tet_j()
{
    int i;
    for (i = 0; i < 4; i++)
        curr_tet.blocks[i].c = TET_COLOR_J;
    switch (curr_tet.r)
    {
        case 0:
            curr_tet.blocks[0].x = curr_tet.x + BLOCK_WIDTH;
            curr_tet.blocks[0].y = curr_tet.y + BLOCK_HEIGHT;
            curr_tet.blocks[1].x = curr_tet.x;
            curr_tet.blocks[1].y = curr_tet.y + BLOCK_HEIGHT;
            curr_tet.blocks[2].x = curr_tet.x + 2*BLOCK_WIDTH;
            curr_tet.blocks[2].y = curr_tet.y + BLOCK_HEIGHT;
            curr_tet.blocks[3].x = curr_tet.x;
            curr_tet.blocks[3].y = curr_tet.y;
            break;
        case 1:
            curr_tet.blocks[0].x = curr_tet.x + BLOCK_WIDTH;
            curr_tet.blocks[0].y = curr_tet.y + BLOCK_HEIGHT;
            curr_tet.blocks[1].x = curr_tet.x + BLOCK_WIDTH;
            curr_tet.blocks[1].y = curr_tet.y;
            curr_tet.blocks[2].x = curr_tet.x + BLOCK_WIDTH;
            curr_tet.blocks[2].y = curr_tet.y + 2*BLOCK_HEIGHT;
            curr_tet.blocks[3].x = curr_tet.x + 2*BLOCK_WIDTH;
            curr_tet.blocks[3].y = curr_tet.y;
            break;
        case 2:
            curr_tet.blocks[0].x = curr_tet.x + BLOCK_WIDTH;
            curr_tet.blocks[0].y = curr_tet.y + BLOCK_HEIGHT;
            curr_tet.blocks[1].x = curr_tet.x;
            curr_tet.blocks[1].y = curr_tet.y + BLOCK_HEIGHT;
            curr_tet.blocks[2].x = curr_tet.x + 2*BLOCK_WIDTH;
            curr_tet.blocks[2].y = curr_tet.y + BLOCK_HEIGHT;
            curr_tet.blocks[3].x = curr_tet.x + 2*BLOCK_WIDTH;
            curr_tet.blocks[3].y = curr_tet.y + 2*BLOCK_HEIGHT;
            break;
        case 3:
            curr_tet.blocks[0].x = curr_tet.x + BLOCK_WIDTH;
            curr_tet.blocks[0].y = curr_tet.y + BLOCK_HEIGHT;
            curr_tet.blocks[1].x = curr_tet.x + BLOCK_WIDTH;
            curr_tet.blocks[1].y = curr_tet.y;
            curr_tet.blocks[2].x = curr_tet.x + BLOCK_WIDTH;
            curr_tet.blocks[2].y = curr_tet.y + 2*BLOCK_HEIGHT;
            curr_tet.blocks[3].x = curr_tet.x;
            curr_tet.blocks[3].y = curr_tet.y + 2*BLOCK_HEIGHT;
            break;
        default:
            break;
    }
}

void set_tet_l()
{
    int i;
    for (i = 0; i < 4; i++)
        curr_tet.blocks[i].c = TET_COLOR_L;
    switch (curr_tet.r)
    {
        case 0:
            curr_tet.blocks[0].x = curr_tet.x + BLOCK_WIDTH;
            curr_tet.blocks[0].y = curr_tet.y + BLOCK_HEIGHT;
            curr_tet.blocks[1].x = curr_tet.x;
            curr_tet.blocks[1].y = curr_tet.y + BLOCK_HEIGHT;
            curr_tet.blocks[2].x = curr_tet.x + 2*BLOCK_WIDTH;
            curr_tet.blocks[2].y = curr_tet.y + BLOCK_HEIGHT;
            curr_tet.blocks[3].x = curr_tet.x + 2*BLOCK_WIDTH;
            curr_tet.blocks[3].y = curr_tet.y;
            break;
        case 1:
            curr_tet.blocks[0].x = curr_tet.x + BLOCK_WIDTH;
            curr_tet.blocks[0].y = curr_tet.y + BLOCK_HEIGHT;
            curr_tet.blocks[1].x = curr_tet.x + BLOCK_WIDTH;
            curr_tet.blocks[1].y = curr_tet.y;
            curr_tet.blocks[2].x = curr_tet.x + BLOCK_WIDTH;
            curr_tet.blocks[2].y = curr_tet.y + 2*BLOCK_HEIGHT;
            curr_tet.blocks[3].x = curr_tet.x + 2*BLOCK_WIDTH;
            curr_tet.blocks[3].y = curr_tet.y + 2*BLOCK_HEIGHT;
            break;
        case 2:
            curr_tet.blocks[0].x = curr_tet.x + BLOCK_WIDTH;
            curr_tet.blocks[0].y = curr_tet.y + BLOCK_HEIGHT;
            curr_tet.blocks[1].x = curr_tet.x;
            curr_tet.blocks[1].y = curr_tet.y + BLOCK_HEIGHT;
            curr_tet.blocks[2].x = curr_tet.x + 2*BLOCK_WIDTH;
            curr_tet.blocks[2].y = curr_tet.y + BLOCK_HEIGHT;
            curr_tet.blocks[3].x = curr_tet.x;
            curr_tet.blocks[3].y = curr_tet.y + 2*BLOCK_HEIGHT;
            break;
        case 3:
            curr_tet.blocks[0].x = curr_tet.x + BLOCK_WIDTH;
            curr_tet.blocks[0].y = curr_tet.y + BLOCK_HEIGHT;
            curr_tet.blocks[1].x = curr_tet.x + BLOCK_WIDTH;
            curr_tet.blocks[1].y = curr_tet.y;
            curr_tet.blocks[2].x = curr_tet.x + BLOCK_WIDTH;
            curr_tet.blocks[2].y = curr_tet.y + 2*BLOCK_HEIGHT;
            curr_tet.blocks[3].x = curr_tet.x;
            curr_tet.blocks[3].y = curr_tet.y;
            break;
        default:
            break;
    }
}
