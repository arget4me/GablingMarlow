#ifndef ANIMATION_MANAGER_HEADER
#define ANIMATION_MANAGER_HEADER

#include <Renderer/opengl_renderer.h>
#define MAX_NUM_CHILDREN 31

typedef struct
{
	glm::mat4 inv_bind_mat;//4 * 8bytes
	unsigned char num_children; //1byte
	unsigned char children[MAX_NUM_CHILDREN]; //31bytes
}Bone;//96bytes

typedef struct
{
	float timestamp;
	glm::vec3 position;
	glm::vec3 size;
	glm::quat orientation;
}Frame;

typedef struct
{
	Mesh mesh;
	unsigned char num_bones;
	unsigned char num_frames;

	Bone* bones;//[num_bones]
	//animation.frames = new Frame[animation.num_bones * animation.num_frames];
	Frame* frames;//num_bones * num_frames -> [Bone0.Frame0, Bone0.Frame1, ... BoneN.Frame0, BoneN.Frame1, ... BoneN.FrameM]
	glm::mat4* animation_transforms;//To be updated and uploaded every frame

}AnimatedMesh;

void update_animation(AnimatedMesh& animation, float timestamp);

#endif // !ANIMATION_MANAGER_HEADER
