

#ifndef DRM_PLANE_HELPER_H
#define DRM_PLANE_HELPER_H

#include <drm/drm_rect.h>
#include <drm/drm_crtc.h>
#include <drm/drm_modeset_helper_vtables.h>
#include <drm/drm_modeset_helper.h>


#define DRM_PLANE_HELPER_NO_SCALING (1<<16)

void drm_primary_helper_destroy(struct drm_plane *plane);
extern const struct drm_plane_funcs drm_primary_helper_funcs;

#endif
