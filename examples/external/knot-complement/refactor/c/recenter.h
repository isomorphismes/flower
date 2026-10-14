#ifndef KC_RECENTER_H
#define KC_RECENTER_H

#include <stddef.h>

typedef struct {
    float x;
    float y;
    float z;
    float w;
} kc_point4;

typedef struct {
    float m[4][4];
} kc_transform;

typedef struct {
    kc_point4 center;
    const kc_transform *neighbors;
    size_t neighbor_count;
} kc_space;

typedef enum {
    KC_OK = 0,
    KC_BAD_INPUT = 1,
    KC_SINGULAR_TRANSFORM = 2,
    KC_NO_CONVERGENCE = 3
} kc_status;

kc_transform kc_identity(void);
kc_transform kc_compose(kc_transform left, kc_transform right);
kc_point4 kc_apply(kc_point4 point, kc_transform transform);
kc_status kc_inverse(kc_transform transform, kc_transform *inverse);
float kc_hyperbolic_distance(kc_point4 a, kc_point4 b);
int kc_needs_hyperbolic_tuneup(kc_transform transform);
kc_transform kc_tune_hyperbolic(kc_transform transform);

kc_status kc_find_nearest_copy(
    const kc_space *space,
    kc_point4 point,
    kc_transform *group_element,
    size_t *steps
);

kc_status kc_recenter_camera(
    const kc_space *space,
    kc_transform camera_to_world,
    kc_transform model_to_world,
    kc_transform *new_camera_to_world,
    kc_transform *group_element,
    size_t *steps
);

#endif
