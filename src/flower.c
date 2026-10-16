#include "flower.h"
#include <math.h>
#include <string.h>

#define PI 3.14159265358979323846f
static FlowerPoint add(FlowerPoint a, FlowerPoint b) { return (FlowerPoint){a.x+b.x,a.y+b.y,a.z+b.z}; }
static FlowerPoint sub(FlowerPoint a, FlowerPoint b) { return (FlowerPoint){a.x-b.x,a.y-b.y,a.z-b.z}; }
static FlowerPoint scale(FlowerPoint a, float s) { return (FlowerPoint){a.x*s,a.y*s,a.z*s}; }
static float dot(FlowerPoint a, FlowerPoint b) { return a.x*b.x+a.y*b.y+a.z*b.z; }
static int vertex(const FlowerNet *net, int ring, int slice) { return ring*net->slices+(slice%net->slices); }

bool flower_net_init(FlowerNet *net, int rings, int slices) {
    if (rings<1 || rings>FLOWER_RINGS || slices<3 || slices>FLOWER_SLICES) return false;
    memset(net,0,sizeof(*net));
    net->rings=rings; net->slices=slices; net->node_count=(rings+1)*slices;
    net->cell_count=rings*slices;
    for (int ring=0;ring<=rings;ring++) for (int slice=0;slice<slices;slice++) {
        int id=vertex(net,ring,slice);
        net->ring[id]=(uint16_t)ring; net->slice[id]=(uint16_t)slice;
        net->boundary[id]=(uint8_t)((ring==0?1:0)|(ring==rings?2:0));
        /* Ordered quadrilateral edge adjacency; diagonals are mechanical only. */
        net->neighbor[id][net->degree[id]++]=(uint16_t)vertex(net,ring,(slice+slices-1)%slices);
        net->neighbor[id][net->degree[id]++]=(uint16_t)vertex(net,ring,slice+1);
        if (ring>0) net->neighbor[id][net->degree[id]++]=(uint16_t)vertex(net,ring-1,slice);
        if (ring<rings) net->neighbor[id][net->degree[id]++]=(uint16_t)vertex(net,ring+1,slice);
        if (ring<rings) {
            uint16_t *cell=net->cell[ring*slices+slice];
            cell[0]=(uint16_t)id; cell[1]=(uint16_t)vertex(net,ring,slice+1);
            cell[2]=(uint16_t)vertex(net,ring+1,slice+1); cell[3]=(uint16_t)vertex(net,ring+1,slice);
        }
    }
    return true;
}
bool flower_net_hops(const FlowerNet *net, int seed, int radius, int16_t *distance) {
    if (seed<0 || seed>=net->node_count || radius<0 || radius>FLOWER_VERTICES) return false;
    for (int i=0;i<net->node_count;i++) distance[i]=-1;
    uint16_t queue[FLOWER_VERTICES]; int read=0, count=1;
    queue[0]=(uint16_t)seed; distance[seed]=0;
    while (read<count) {
        int node=queue[read++];
        if (distance[node]>=radius) continue;
        for (int i=0;i<net->degree[node];i++) {
            int next=net->neighbor[node][i];
            if (distance[next]>=0) continue;
            distance[next]=(int16_t)(distance[node]+1); queue[count++]=(uint16_t)next;
        }
    }
    return true;
}
FlowerPolicy flower_policy_default(void) { return (FlowerPolicy){.rate=0.28f,.falloff=2,.edge_bias=0.8f,.radial_fraction=0,.hop_radius=4}; }
bool flower_policy_valid(FlowerPolicy p) {
    return isfinite(p.rate) && p.rate>=0 && p.rate<=2 && isfinite(p.falloff) && p.falloff>=0 && p.falloff<=8 &&
        isfinite(p.edge_bias) && p.edge_bias>=0 && p.edge_bias<=1 &&
        isfinite(p.radial_fraction) && p.radial_fraction>=0 && p.radial_fraction<=1 && p.hop_radius>=0 && p.hop_radius<=16;
}
static bool valid_hit(FlowerHit hit) {
    return hit.hit && isfinite(hit.radial) && isfinite(hit.angle) && hit.radial>=0 && hit.radial<=1 && hit.node<FLOWER_VERTICES;
}
static void link_add(FlowerSkin *skin, int first, int second, float stiffness) {
    FlowerPoint delta=sub(skin->reference[second],skin->reference[first]);
    FlowerPoint middle=add(skin->reference[first],skin->reference[second]);
    float radius_squared=middle.x*middle.x+middle.z*middle.z, length_squared=dot(delta,delta);
    float tangent=radius_squared>0 ? (-delta.x*middle.z+delta.z*middle.x)/sqrtf(radius_squared) : 0;
    FlowerLink *link=&skin->link[skin->link_count++];
    *link=(FlowerLink){(uint16_t)first,(uint16_t)second,sqrtf(length_squared),fminf(1,tangent*tangent/length_squared),stiffness};
}
void flower_init(Flower *flower) {
    memset(flower,0,sizeof(*flower));
    flower_net_init(&flower->net,FLOWER_RINGS,FLOWER_SLICES);
    FlowerNet *net=&flower->net; FlowerSkin *skin=&flower->skin;
    for (int ring=0;ring<=net->rings;ring++) for (int slice=0;slice<net->slices;slice++) {
        int index=vertex(net,ring,slice);
        float radial=(float)ring/net->rings, angle=2*PI*slice/net->slices;
        float radius=0.22f+radial*(1.5f+0.32f*cosf(5*angle));
        uint32_t random=(uint32_t)(index+1)*UINT32_C(2654435761);
        float seed=((float)(random>>16)/65535.0f-0.5f)*0.006f*radial;
        skin->reference[index]=skin->position[index]=(FlowerPoint){radius*cosf(angle),0.16f*radial*radial+seed,radius*sinf(angle)};
    }
    for (int ring=0;ring<=net->rings;ring++) for (int slice=0;slice<net->slices;slice++) {
        int first=vertex(net,ring,slice), next=vertex(net,ring,slice+1);
        link_add(skin,first,next,0.8f);
        link_add(skin,first,vertex(net,ring,slice+2),0.035f);
        if (ring<net->rings) {
            int outer=vertex(net,ring+1,slice), diagonal=vertex(net,ring+1,slice+1);
            link_add(skin,first,outer,0.8f); link_add(skin,first,diagonal,0.65f); link_add(skin,next,outer,0.65f);
        }
        if (ring<net->rings-1) link_add(skin,first,vertex(net,ring+2,slice),0.035f);
    }
}
void flower_hold_init(FlowerHold *hold) { *hold=(FlowerHold){.owner=-1}; }
void flower_hold_cancel(FlowerHold *hold) { *hold=(FlowerHold){.blocked=true,.owner=-1}; }
void flower_hold_all_released(FlowerHold *hold) { flower_hold_init(hold); }
bool flower_hold_begin(FlowerHold *hold, int pointer, bool fresh_press, bool focused, FlowerHit hit) {
    if (hold->captured || hold->blocked || !fresh_press || !focused || !valid_hit(hit)) return false;
    *hold=(FlowerHold){.captured=true,.owner=pointer}; return true;
}
void flower_hold_release(FlowerHold *hold, int pointer) {
    if (hold->captured && hold->owner==pointer) flower_hold_cancel(hold);
}
static void grow_intrinsic_geometry(const FlowerNet *net, FlowerSkin *skin,
                                    const int16_t *distance, const FlowerPolicy *policy) {
    for (int index=0;index<net->node_count;index++) {
        if (net->boundary[index]&1 || distance[index]<0) continue;
        float weight=powf(1-(float)distance[index]/(policy->hop_radius+1),policy->falloff);
        float radial=(float)net->ring[index]/net->rings;
        float gain=policy->rate*(float)FLOWER_STEP*weight*(1-policy->edge_bias+policy->edge_bias*radial*radial);
        skin->growth[index]=fminf(1.5f,skin->growth[index]+gain);
        skin->radial_growth[index]=fminf(1.5f,skin->radial_growth[index]+gain*policy->radial_fraction);
    }
}
static float preferred_link_length(const FlowerSkin *skin, FlowerLink link) {
    float tangent=1+0.5f*(skin->growth[link.first]+skin->growth[link.second]);
    float radial=1+0.5f*(skin->radial_growth[link.first]+skin->radial_growth[link.second]);
    return link.length*sqrtf(tangent*tangent*link.tangent_squared+radial*radial*(1-link.tangent_squared));
}
static void relax_material_link(const FlowerNet *net, FlowerSkin *skin, FlowerLink link) {
    FlowerPoint delta=sub(skin->position[link.second],skin->position[link.first]);
    float length=sqrtf(dot(delta,delta));
    int first_free=!(net->boundary[link.first]&1), second_free=!(net->boundary[link.second]&1);
    int free_count=first_free+second_free;
    if (length<1e-8f || !free_count) return;
    float preferred=preferred_link_length(skin,link);
    FlowerPoint correction=scale(delta,link.stiffness*(length-preferred)/(length*free_count));
    if (first_free) skin->position[link.first]=add(skin->position[link.first],correction);
    if (second_free) skin->position[link.second]=sub(skin->position[link.second],correction);
}
static void relax_skin(const FlowerNet *net, FlowerSkin *skin) {
    /* Ordered spring-sheet relaxation; not a calibrated shell or collision solver. */
    for (int pass=0;pass<8;pass++) for (int order=0;order<skin->link_count;order++) {
        int index=(pass&1)? skin->link_count-1-order : order;
        relax_material_link(net,skin,skin->link[index]);
    }
}
static void simulation_step(Flower *flower, FlowerHit hit, const FlowerPolicy *policy) {
    int16_t distance[FLOWER_VERTICES];
    if (!flower_net_hops(&flower->net,hit.node,policy->hop_radius,distance)) return;
    grow_intrinsic_geometry(&flower->net,&flower->skin,distance,policy);
    relax_skin(&flower->net,&flower->skin);
    flower->skin.steps++;
}
bool flower_hold_frame_policy(Flower *flower, FlowerHold *hold, int pointer, bool down,
                              bool focused, FlowerHit hit, double frame_seconds, const FlowerPolicy *policy) {
    if (!focused || !isfinite(frame_seconds) || frame_seconds<0 || frame_seconds>FLOWER_FRAME_LIMIT || !policy || !flower_policy_valid(*policy)) {
        flower_hold_cancel(hold); return false;
    }
    if (!hold->captured || hold->blocked || pointer!=hold->owner) return false;
    if (!down) { flower_hold_release(hold,pointer); return false; }
    if (!valid_hit(hit) || hit.node>=flower->net.node_count) { hold->remainder=0; return false; }
    hold->remainder+=fmin(frame_seconds,FLOWER_CATCHUP_LIMIT);
    bool changed=false;
    while (hold->remainder+1e-12>=FLOWER_STEP) {
        simulation_step(flower,hit,policy);
        hold->remainder-=FLOWER_STEP;
        if (hold->remainder<0) hold->remainder=0;
        changed=true;
    }
    return changed;
}
bool flower_hold_frame(Flower *flower, FlowerHold *hold, int pointer, bool down,
                       bool focused, FlowerHit hit, double seconds) {
    FlowerPolicy policy=flower_policy_default();
    return flower_hold_frame_policy(flower,hold,pointer,down,focused,hit,seconds,&policy);
}
