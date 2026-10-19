#include "recenter.h"

#include <math.h>

static float minkowski_dot(kc_point4 a, kc_point4 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z - a.w * b.w;
}

static kc_point4 scale_point(float scale, kc_point4 point)
{
    kc_point4 out = {
        scale * point.x,
        scale * point.y,
        scale * point.z,
        scale * point.w
    };
    return out;
}

static kc_point4 subtract_point(kc_point4 a, kc_point4 b)
{
    kc_point4 out = {a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};
    return out;
}

static kc_point4 normalize_hyperbolic(kc_point4 point)
{
    float length = sqrtf(fabsf(minkowski_dot(point, point)));
    if (length == 0.0f) {
        return point;
    }
    return scale_point(1.0f / length, point);
}

static kc_point4 gram_schmidt(kc_point4 base, kc_point4 point)
{
    float denominator = minkowski_dot(base, base);
    if (denominator == 0.0f) {
        return point;
    }
    return subtract_point(
        point,
        scale_point(minkowski_dot(base, point) / denominator, base)
    );
}

static kc_point4 row(kc_transform transform, size_t index)
{
    kc_point4 out = {
        transform.m[index][0],
        transform.m[index][1],
        transform.m[index][2],
        transform.m[index][3]
    };
    return out;
}

static void set_row(kc_transform *transform, size_t index, kc_point4 point)
{
    transform->m[index][0] = point.x;
    transform->m[index][1] = point.y;
    transform->m[index][2] = point.z;
    transform->m[index][3] = point.w;
}

kc_transform kc_identity(void)
{
    kc_transform out = {0};
    for (size_t i = 0; i < 4; ++i) {
        out.m[i][i] = 1.0f;
    }
    return out;
}

kc_transform kc_compose(kc_transform left, kc_transform right)
{
    kc_transform out = {0};
    for (size_t i = 0; i < 4; ++i) {
        for (size_t j = 0; j < 4; ++j) {
            for (size_t k = 0; k < 4; ++k) {
                out.m[i][j] += left.m[i][k] * right.m[k][j];
            }
        }
    }
    return out;
}

kc_point4 kc_apply(kc_point4 point, kc_transform transform)
{
    kc_point4 out = {
        point.x * transform.m[0][0] + point.y * transform.m[1][0] +
            point.z * transform.m[2][0] + point.w * transform.m[3][0],
        point.x * transform.m[0][1] + point.y * transform.m[1][1] +
            point.z * transform.m[2][1] + point.w * transform.m[3][1],
        point.x * transform.m[0][2] + point.y * transform.m[1][2] +
            point.z * transform.m[2][2] + point.w * transform.m[3][2],
        point.x * transform.m[0][3] + point.y * transform.m[1][3] +
            point.z * transform.m[2][3] + point.w * transform.m[3][3]
    };
    return out;
}

kc_status kc_inverse(kc_transform transform, kc_transform *inverse)
{
    if (inverse == NULL) {
        return KC_BAD_INPUT;
    }

    kc_transform work = transform;
    kc_transform out = kc_identity();

    for (size_t i = 0; i < 4; ++i) {
        size_t largest = i;
        float largest_square = work.m[i][i] * work.m[i][i];
        for (size_t j = i + 1; j < 4; ++j) {
            float square = work.m[j][i] * work.m[j][i];
            if (square > largest_square) {
                largest = j;
                largest_square = square;
            }
        }
        if (largest_square == 0.0f) {
            return KC_SINGULAR_TRANSFORM;
        }

        if (largest != i) {
            for (size_t k = 0; k < 4; ++k) {
                float tmp = work.m[i][k];
                work.m[i][k] = work.m[largest][k];
                work.m[largest][k] = tmp;

                tmp = out.m[i][k];
                out.m[i][k] = out.m[largest][k];
                out.m[largest][k] = tmp;
            }
        }

        for (size_t j = i + 1; j < 4; ++j) {
            float factor = work.m[j][i] / work.m[i][i];
            for (size_t k = 0; k < 4; ++k) {
                work.m[j][k] -= factor * work.m[i][k];
                out.m[j][k] -= factor * out.m[i][k];
            }
        }
    }

    for (size_t i = 0; i < 4; ++i) {
        float factor = work.m[i][i];
        if (factor == 0.0f) {
            return KC_SINGULAR_TRANSFORM;
        }
        for (size_t k = 0; k < 4; ++k) {
            work.m[i][k] /= factor;
            out.m[i][k] /= factor;
        }
    }

    for (int i = 3; i >= 0; --i) {
        for (int j = i - 1; j >= 0; --j) {
            float factor = work.m[j][i];
            for (size_t k = 0; k < 4; ++k) {
                work.m[j][k] -= factor * work.m[i][k];
                out.m[j][k] -= factor * out.m[i][k];
            }
        }
    }

    *inverse = out;
    return KC_OK;
}

float kc_hyperbolic_distance(kc_point4 a, kc_point4 b)
{
    float aa = minkowski_dot(a, a);
    float bb = minkowski_dot(b, b);
    float ab = minkowski_dot(a, b);
    float ratio = fabsf(ab / sqrtf(aa * bb));
    if (ratio < 1.0f) {
        ratio = 1.0f;
    }
    return acoshf(ratio);
}

int kc_needs_hyperbolic_tuneup(kc_transform transform)
{
    for (size_t i = 0; i < 4; ++i) {
        for (size_t j = i; j < 4; ++j) {
            float d = transform.m[i][0] * transform.m[j][0] +
                transform.m[i][1] * transform.m[j][1] +
                transform.m[i][2] * transform.m[j][2] -
                transform.m[i][3] * transform.m[j][3];
            if (i == 3) {
                d *= -1.0f;
            }
            if (fabsf(d - (float)(i == j)) > 0.01f) {
                return 1;
            }
        }
    }
    return 0;
}

kc_transform kc_tune_hyperbolic(kc_transform transform)
{
    kc_point4 r0 = normalize_hyperbolic(row(transform, 0));

    kc_point4 r1 = gram_schmidt(r0, row(transform, 1));
    r1 = normalize_hyperbolic(r1);

    kc_point4 r2 = gram_schmidt(r0, row(transform, 2));
    r2 = gram_schmidt(r1, r2);
    r2 = normalize_hyperbolic(r2);

    kc_point4 r3 = gram_schmidt(r0, row(transform, 3));
    r3 = gram_schmidt(r1, r3);
    r3 = gram_schmidt(r2, r3);
    r3 = normalize_hyperbolic(r3);

    set_row(&transform, 0, r0);
    set_row(&transform, 1, r1);
    set_row(&transform, 2, r2);
    set_row(&transform, 3, r3);
    return transform;
}

kc_status kc_find_nearest_copy(
    const kc_space *space,
    kc_point4 point,
    kc_transform *group_element,
    size_t *steps
)
{
    if (space == NULL || group_element == NULL || space->neighbors == NULL ||
        space->neighbor_count == 0) {
        return KC_BAD_INPUT;
    }

    kc_transform closest = kc_identity();
    kc_point4 current = point;

    for (size_t count = 0; count < 1000; ++count) {
        size_t nearest = 0;
        float nearest_distance = 0.0f;

        for (size_t i = 0; i < space->neighbor_count; ++i) {
            kc_point4 image = kc_apply(space->center, space->neighbors[i]);
            float distance = kc_hyperbolic_distance(current, image);
            if (i == 0 || distance < nearest_distance) {
                nearest = i;
                nearest_distance = distance;
            }
        }

        if (nearest == 0) {
            *group_element = closest;
            if (steps != NULL) {
                *steps = count;
            }
            return KC_OK;
        }

        closest = kc_compose(space->neighbors[nearest], closest);

        kc_transform inverse;
        kc_status status = kc_inverse(closest, &inverse);
        if (status != KC_OK) {
            return status;
        }
        current = kc_apply(point, inverse);
    }

    *group_element = closest;
    if (steps != NULL) {
        *steps = 1000;
    }
    return KC_NO_CONVERGENCE;
}

kc_status kc_recenter_camera(
    const kc_space *space,
    kc_transform camera_to_world,
    kc_transform model_to_world,
    kc_transform *new_camera_to_world,
    kc_transform *group_element,
    size_t *steps
)
{
    if (space == NULL || new_camera_to_world == NULL || group_element == NULL) {
        return KC_BAD_INPUT;
    }

    kc_transform world_to_camera;
    kc_transform world_to_model;
    kc_status status = kc_inverse(camera_to_world, &world_to_camera);
    if (status != KC_OK) {
        return status;
    }
    status = kc_inverse(model_to_world, &world_to_model);
    if (status != KC_OK) {
        return status;
    }

    kc_transform model_to_camera = kc_compose(model_to_world, world_to_camera);
    kc_transform camera_to_model;
    status = kc_inverse(model_to_camera, &camera_to_model);
    if (status != KC_OK) {
        return status;
    }

    const kc_point4 origin = {0.0f, 0.0f, 0.0f, 1.0f};
    kc_point4 camera_position = kc_apply(origin, camera_to_model);

    kc_transform closest;
    status = kc_find_nearest_copy(space, camera_position, &closest, steps);
    if (status != KC_OK) {
        return status;
    }

    kc_transform inverse_closest;
    status = kc_inverse(closest, &inverse_closest);
    if (status != KC_OK) {
        return status;
    }

    kc_transform in_world = kc_compose(
        world_to_model,
        kc_compose(inverse_closest, model_to_world)
    );
    kc_transform recentered = kc_compose(camera_to_world, in_world);

    if (kc_needs_hyperbolic_tuneup(recentered)) {
        recentered = kc_tune_hyperbolic(recentered);
    }

    *new_camera_to_world = recentered;
    *group_element = closest;
    return KC_OK;
}
