
#include <drm/drmP.h>
#include <drm.h>
#include <drm_mm.h>
#include "radeon_drm.h"
#include "radeon.h"
#include "radeon_object.h"
#include "display.h"
#include "drm_fb_helper.h"

struct radeon_fbdev {
    struct drm_fb_helper        helper;
    struct radeon_framebuffer   rfb;
    struct list_head fbdev_list;
    struct radeon_device        *rdev;
};

struct radeon_fbdev *kos_rfbdev;


static cursor_t*  __stdcall select_cursor_kms(cursor_t *cursor);
static void       __stdcall move_cursor_kms(cursor_t *cursor, int x, int y);

int radeon_align_pitch(struct radeon_device *rdev, int width, int bpp, bool tiled);

void disable_mouse(void);

static void radeon_show_cursor_kms(struct drm_crtc *crtc)
{
    struct radeon_crtc *radeon_crtc = to_radeon_crtc(crtc);
    struct radeon_device *rdev = crtc->dev->dev_private;

    if (ASIC_IS_AVIVO(rdev)) {
        WREG32(RADEON_MM_INDEX, AVIVO_D1CUR_CONTROL + radeon_crtc->crtc_offset);
        WREG32(RADEON_MM_DATA, AVIVO_D1CURSOR_EN |
                 (AVIVO_D1CURSOR_MODE_24BPP << AVIVO_D1CURSOR_MODE_SHIFT));
    } else {
        switch (radeon_crtc->crtc_id) {
        case 0:
            WREG32(RADEON_MM_INDEX, RADEON_CRTC_GEN_CNTL);
            break;
        case 1:
            WREG32(RADEON_MM_INDEX, RADEON_CRTC2_GEN_CNTL);
            break;
        default:
            return;
        }

        WREG32_P(RADEON_MM_DATA, (RADEON_CRTC_CUR_EN |
                      (RADEON_CRTC_CUR_MODE_24BPP << RADEON_CRTC_CUR_MODE_SHIFT)),
             ~(RADEON_CRTC_CUR_EN | RADEON_CRTC_CUR_MODE_MASK));
    }
}

static void radeon_lock_cursor_kms(struct drm_crtc *crtc, bool lock)
{
    struct radeon_device *rdev = crtc->dev->dev_private;
    struct radeon_crtc *radeon_crtc = to_radeon_crtc(crtc);
    uint32_t cur_lock;

    if (ASIC_IS_AVIVO(rdev)) {
        cur_lock = RREG32(AVIVO_D1CUR_UPDATE + radeon_crtc->crtc_offset);
        if (lock)
            cur_lock |= AVIVO_D1CURSOR_UPDATE_LOCK;
        else
            cur_lock &= ~AVIVO_D1CURSOR_UPDATE_LOCK;
        WREG32(AVIVO_D1CUR_UPDATE + radeon_crtc->crtc_offset, cur_lock);
    } else {
        cur_lock = RREG32(RADEON_CUR_OFFSET + radeon_crtc->crtc_offset);
        if (lock)
            cur_lock |= RADEON_CUR_LOCK;
        else
            cur_lock &= ~RADEON_CUR_LOCK;
        WREG32(RADEON_CUR_OFFSET + radeon_crtc->crtc_offset, cur_lock);
    }
}

cursor_t* __stdcall select_cursor_kms(cursor_t *cursor)
{
    struct radeon_device *rdev;
    struct radeon_crtc   *radeon_crtc;
    cursor_t *old;
    uint32_t  gpu_addr;

    rdev = (struct radeon_device *)rdisplay->ddev->dev_private;
    radeon_crtc = to_radeon_crtc(rdisplay->crtc);

    old = rdisplay->cursor;

    rdisplay->cursor = cursor;
    gpu_addr = radeon_bo_gpu_offset(cursor->robj);

    if (ASIC_IS_AVIVO(rdev))
        WREG32(AVIVO_D1CUR_SURFACE_ADDRESS + radeon_crtc->crtc_offset, gpu_addr);
    else {
        radeon_crtc->legacy_cursor_offset = gpu_addr - rdev->mc.vram_start;
        /* offset is from DISP(2)_BASE_ADDRESS */
        WREG32(RADEON_CUR_OFFSET + radeon_crtc->crtc_offset, radeon_crtc->legacy_cursor_offset);
    }

    return old;
};

void __stdcall move_cursor_kms(cursor_t *cursor, int x, int y)
{
    struct radeon_device *rdev;
    rdev = (struct radeon_device *)rdisplay->ddev->dev_private;
    struct drm_crtc *crtc = rdisplay->crtc;
    struct radeon_crtc *radeon_crtc = to_radeon_crtc(crtc);

    int hot_x = cursor->hot_x;
    int hot_y = cursor->hot_y;

    radeon_lock_cursor_kms(crtc, true);
    if (ASIC_IS_AVIVO(rdev))
    {
        int w = 32;
        int i = 0;
        struct drm_crtc *crtc_p;

        /* avivo cursor are offset into the total surface */
//        x += crtc->x;
//        y += crtc->y;

//        DRM_DEBUG("x %d y %d c->x %d c->y %d\n", x, y, crtc->x, crtc->y);
#if 0
        /* avivo cursor image can't end on 128 pixel boundry or
         * go past the end of the frame if both crtcs are enabled
         */
        list_for_each_entry(crtc_p, &crtc->dev->mode_config.crtc_list, head) {
            if (crtc_p->enabled)
                i++;
        }
        if (i > 1) {
            int cursor_end, frame_end;

            cursor_end = x + w;
            frame_end = crtc->x + crtc->mode.crtc_hdisplay;
            if (cursor_end >= frame_end) {
                w = w - (cursor_end - frame_end);
                if (!(frame_end & 0x7f))
                    w--;
            } else {
                if (!(cursor_end & 0x7f))
                    w--;
            }
            if (w <= 0)
                w = 1;
        }
#endif
        WREG32(AVIVO_D1CUR_POSITION + radeon_crtc->crtc_offset,
               (x << 16) | y);
        WREG32(AVIVO_D1CUR_HOT_SPOT + radeon_crtc->crtc_offset,
               (hot_x << 16) | hot_y);
        WREG32(AVIVO_D1CUR_SIZE + radeon_crtc->crtc_offset,
               ((w - 1) << 16) | 31);
    } else {
        if (crtc->mode.flags & DRM_MODE_FLAG_DBLSCAN)
            y *= 2;

        uint32_t  gpu_addr;
        int       xorg =0, yorg=0;

        x = x - hot_x;
        y = y - hot_y;

        if( x < 0 )
        {
            xorg = -x + 1;
            x = 0;
        }

        if( y < 0 )
        {
            yorg = -hot_y + 1;
            y = 0;
        };

        WREG32(RADEON_CUR_HORZ_VERT_OFF,
               (RADEON_CUR_LOCK | (xorg << 16) | yorg ));
        WREG32(RADEON_CUR_HORZ_VERT_POSN,
               (RADEON_CUR_LOCK | (x << 16) | y));

        gpu_addr = radeon_bo_gpu_offset(cursor->robj);

        /* offset is from DISP(2)_BASE_ADDRESS */
        WREG32(RADEON_CUR_OFFSET,
         (gpu_addr - rdev->mc.vram_start + (yorg * 256)));
    }
    radeon_lock_cursor_kms(crtc, false);
}

static char *manufacturer_name(unsigned char *x)
{
    static char name[4];

    name[0] = ((x[0] & 0x7C) >> 2) + '@';
    name[1] = ((x[0] & 0x03) << 3) + ((x[1] & 0xE0) >> 5) + '@';
    name[2] = (x[1] & 0x1F) + '@';
    name[3] = 0;

    return name;
}

bool set_mode(struct drm_device *dev, struct drm_connector *connector,
              videomode_t *reqmode, bool strict)
{
    struct drm_display_mode  *mode = NULL, *tmpmode;

    struct drm_fb_helper *fb_helper;

    fb_helper = &kos_rfbdev->helper;


    bool ret = false;

    ENTER();

    dbgprintf("width %d height %d vrefresh %d\n",
               reqmode->width, reqmode->height, reqmode->freq);

    list_for_each_entry(tmpmode, &connector->modes, head)
    {
        if( (drm_mode_width(tmpmode)    == reqmode->width)  &&
            (drm_mode_height(tmpmode)   == reqmode->height) &&
            (drm_mode_vrefresh(tmpmode) == reqmode->freq) )
        {
            mode = tmpmode;
            goto do_set;
        }
    };

    if( (mode == NULL) && (strict == false) )
    {
        list_for_each_entry(tmpmode, &connector->modes, head)
        {
            if( (drm_mode_width(tmpmode)  == reqmode->width)  &&
                (drm_mode_height(tmpmode) == reqmode->height) )
            {
                mode = tmpmode;
                goto do_set;
            }
        };
    };

do_set:

    if( mode != NULL )
    {
        struct drm_framebuffer   *fb;
        struct drm_encoder       *encoder;
        struct drm_crtc          *crtc;

//        char  con_edid[128];
        char *con_name;
        char *enc_name;

        encoder = connector->encoder;
        crtc = encoder->crtc;

//        fb = list_first_entry(&dev->mode_config.fb_kernel_list,
//                              struct drm_framebuffer, filp_head);

//        memcpy(con_edid, connector->edid_blob_ptr->data, 128);

//        dbgprintf("Manufacturer: %s Model %x Serial Number %u\n",
//        manufacturer_name(con_edid + 0x08),
//        (unsigned short)(con_edid[0x0A] + (con_edid[0x0B] << 8)),
//        (unsigned int)(con_edid[0x0C] + (con_edid[0x0D] << 8)
//            + (con_edid[0x0E] << 16) + (con_edid[0x0F] << 24)));

        con_name = drm_get_connector_name(connector);
        enc_name = drm_get_encoder_name(encoder);

        dbgprintf("set mode %d %d connector %s encoder %s\n",
                   reqmode->width, reqmode->height, con_name, enc_name);

        fb = fb_helper->fb;

        fb->width  = reqmode->width;
        fb->height = reqmode->height;
        fb->pitch  = radeon_align_pitch(dev->dev_private, reqmode->width, 32, false) * ((32 + 1) / 8);
        fb->bits_per_pixel = 32;

        crtc->fb = fb;
        crtc->enabled = true;
        rdisplay->crtc = crtc;

        ret = drm_crtc_helper_set_mode(crtc, mode, 0, 0, fb);

        select_cursor_kms(rdisplay->cursor);
        radeon_show_cursor_kms(crtc);

        if (ret == true)
        {
            rdisplay->width    = fb->width;
            rdisplay->height   = fb->height;
            rdisplay->pitch    = fb->pitch;
            rdisplay->vrefresh = drm_mode_vrefresh(mode);

            sysSetScreen(fb->width, fb->height, fb->pitch);

            dbgprintf("new mode %d x %d pitch %d\n",
                       fb->width, fb->height, fb->pitch);
        }
        else
            DRM_ERROR("failed to set mode %d_%d on crtc %p\n",
                       fb->width, fb->height, crtc);
    }

    LEAVE();
    return ret;
};

static int count_connector_modes(struct drm_connector* connector)
{
    struct drm_display_mode  *mode;
    int count = 0;

    list_for_each_entry(mode, &connector->modes, head)
    {
        count++;
    };
    return count;
};

static struct drm_connector* get_def_connector(struct drm_device *dev)
{
    struct drm_connector  *connector;
    struct drm_connector_helper_funcs *connector_funcs;

    struct drm_connector  *def_connector = NULL;

    list_for_each_entry(connector, &dev->mode_config.connector_list, head)
    {
        struct drm_encoder  *encoder;
        struct drm_crtc     *crtc;

        if( connector->status != connector_status_connected)
            continue;

        connector_funcs = connector->helper_private;
        encoder = connector_funcs->best_encoder(connector);
        if( encoder == NULL)
            continue;

        connector->encoder = encoder;

        crtc = encoder->crtc;

        dbgprintf("CONNECTOR %x ID:  %d status %d encoder %x\n crtc %x",
                   connector, connector->base.id,
                   connector->status, connector->encoder,
                   crtc);

//        if (crtc == NULL)
//            continue;

        def_connector = connector;

        break;
    };

    return def_connector;
};



bool init_display_kms(struct radeon_device *rdev, videomode_t *usermode)
{
    struct drm_device   *dev;

    cursor_t            *cursor;
    bool                 retval = false;
    u32_t                ifl;

    struct radeon_fbdev  *rfbdev;
    struct drm_fb_helper *fb_helper;

    int i;

    ENTER();

    rdisplay = GetDisplay();

    dev = rdisplay->ddev = rdev->ddev;

    ifl = safe_cli();
    {
        list_for_each_entry(cursor, &rdisplay->cursors, list)
        {
            init_cursor(cursor);
        };
    };
    safe_sti(ifl);



    rfbdev    = rdev->mode_info.rfbdev;
    fb_helper = &rfbdev->helper;


//    for (i = 0; i < fb_helper->crtc_count; i++)
//    {
        struct drm_mode_set *mode_set = &fb_helper->crtc_info[0].mode_set;
        struct drm_crtc *crtc;
        struct drm_display_mode *mode;

        crtc = mode_set->crtc;

//        if (!crtc->enabled)
//            continue;

        mode = mode_set->mode;

        dbgprintf("crtc %d width %d height %d vrefresh %d\n",
               crtc->base.id,
               drm_mode_width(mode), drm_mode_height(mode),
               drm_mode_vrefresh(mode));
//    }


    rdisplay->connector = get_def_connector(dev);
    if( rdisplay->connector == 0 )
    {
        dbgprintf("no active connectors\n");
        return false;
    };


    rdisplay->crtc = rdisplay->connector->encoder->crtc = crtc;

    rdisplay->supported_modes = count_connector_modes(rdisplay->connector);

    dbgprintf("current mode %d x %d x %d\n",
              rdisplay->width, rdisplay->height, rdisplay->vrefresh);
    dbgprintf("user mode mode %d x %d x %d\n",
              usermode->width, usermode->height, usermode->freq);

    if( (usermode->width  != 0) &&
        (usermode->height != 0) &&
        ( (usermode->width  != rdisplay->width)  ||
          (usermode->height != rdisplay->height) ||
          (usermode->freq   != rdisplay->vrefresh) ) )
    {

        retval = set_mode(dev, rdisplay->connector, usermode, false);
    }

    ifl = safe_cli();
    {
        rdisplay->restore_cursor(0,0);
        rdisplay->init_cursor    = init_cursor;
        rdisplay->select_cursor  = select_cursor_kms;
        rdisplay->show_cursor    = NULL;
        rdisplay->move_cursor    = move_cursor_kms;
        rdisplay->restore_cursor = restore_cursor;
        rdisplay->disable_mouse  = disable_mouse;

        select_cursor_kms(rdisplay->cursor);
        radeon_show_cursor_kms(rdisplay->crtc);
    };
    safe_sti(ifl);

    LEAVE();

    return retval;
};

int get_modes(videomode_t *mode, int *count)
{
    int err = -1;

    ENTER();

    dbgprintf("mode %x count %d\n", mode, *count);

    if( *count == 0 )
    {
        *count = rdisplay->supported_modes;
        err = 0;
    }
    else if( mode != NULL )
    {
        struct drm_display_mode  *drmmode;
        int i = 0;

        if( *count > rdisplay->supported_modes)
            *count = rdisplay->supported_modes;

        list_for_each_entry(drmmode, &rdisplay->connector->modes, head)
        {
            if( i < *count)
            {
                mode->width  = drm_mode_width(drmmode);
                mode->height = drm_mode_height(drmmode);
                mode->bpp    = 32;
                mode->freq   = drm_mode_vrefresh(drmmode);
                i++;
                mode++;
            }
            else break;
        };
        *count = i;
        err = 0;
    };
    LEAVE();
    return err;
}

int set_user_mode(videomode_t *mode)
{
    int err = -1;

    ENTER();

    dbgprintf("width %d height %d vrefresh %d\n",
               mode->width, mode->height, mode->freq);

    if( (mode->width  != 0)  &&
        (mode->height != 0)  &&
        (mode->freq   != 0 ) &&
        ( (mode->width   != rdisplay->width)  ||
          (mode->height  != rdisplay->height) ||
          (mode->freq    != rdisplay->vrefresh) ) )
    {
        if( set_mode(rdisplay->ddev, rdisplay->connector, mode, true) )
            err = 0;
    };

    LEAVE();
    return err;
};



int radeonfb_create_object(struct radeon_fbdev *rfbdev,
                     struct drm_mode_fb_cmd *mode_cmd,
                     struct drm_gem_object **gobj_p)
{
    struct radeon_device *rdev = rfbdev->rdev;
    struct drm_gem_object *gobj = NULL;
    struct radeon_bo *rbo = NULL;
    bool fb_tiled = false; /* useful for testing */
    u32 tiling_flags = 0;
    int ret;
    int aligned_size, size;
    int height = mode_cmd->height;

    static struct radeon_bo kos_bo;
    static struct drm_mm_node  vm_node;

    /* need to align pitch with crtc limits */
    mode_cmd->pitch = radeon_align_pitch(rdev, mode_cmd->width, mode_cmd->bpp, fb_tiled) * ((mode_cmd->bpp + 1) / 8);

    if (rdev->family >= CHIP_R600)
        height = ALIGN(mode_cmd->height, 8);
    size = mode_cmd->pitch * height;
    aligned_size = ALIGN(size, PAGE_SIZE);

    ret = drm_gem_object_init(rdev->ddev, &kos_bo.gem_base, aligned_size);
    if (unlikely(ret)) {
        return ret;
    }

    kos_bo.rdev = rdev;
    kos_bo.gem_base.driver_private = NULL;
    kos_bo.surface_reg = -1;
    kos_bo.domain = RADEON_GEM_DOMAIN_VRAM;

    INIT_LIST_HEAD(&kos_bo.list);

    gobj = &kos_bo.gem_base;
    rbo = gem_to_radeon_bo(gobj);

    if (fb_tiled)
        tiling_flags = RADEON_TILING_MACRO;

    if (tiling_flags) {
        rbo->tiling_flags = tiling_flags | RADEON_TILING_SURFACE;
        rbo->pitch = mode_cmd->pitch;
    }

    vm_node.size = 0xC00000 >> 12;
    vm_node.start = 0;
    vm_node.mm = NULL;

    rbo->tbo.vm_node = &vm_node;
    rbo->tbo.offset  = rbo->tbo.vm_node->start << PAGE_SHIFT;
    rbo->tbo.offset += (u64)rbo->rdev->mc.vram_start;
    rbo->kptr        = (void*)0xFE000000;
    rbo->pin_count   = 1;

//    if (fb_tiled)
//        radeon_bo_check_tiling(rbo, 0, 0);

    *gobj_p = gobj;
    return 0;
}




