#include "animation_manager.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <limits>
#include <cmath>
#include <Utils/logfile.h>

glm::mat4 get_frame_matrix(Frame& frame)
{
	glm::mat4 mat(1.0f);

	mat = glm::translate(mat, frame.position);
	mat = mat * glm::toMat4(frame.orientation);
	//mat = glm::scale(mat, frame.size);

	return mat;
}

void calc_animation(AnimatedMesh& animation, float timestamp, int bone_index, glm::mat4 parent_transform)
{
	float min_timestamp = std::numeric_limits<int>::max();
	float max_timestamp = -1.0f;

	for (int i = 0; i < animation.num_frames; i++)
	{
		float t = animation.frames[bone_index * animation.num_frames + i].timestamp;
		if (t > max_timestamp)
		{
			max_timestamp = t;
		}
		if (t < min_timestamp)
		{
			min_timestamp = t;
		}
	}
	float animation_length = max_timestamp - min_timestamp;
	//@TODO: Animation length can be precomputed


	timestamp = min_timestamp +fmod(timestamp, animation_length);
	//DEBUG_LOG("t= " << timestamp << "\t" << animation_length<<"(" << min_timestamp << ", " << max_timestamp<<")" << "\n");
	int frame_index = animation.num_frames - 1;
	for (int i = 0; i < animation.num_frames; i++)
	{
		float t = animation.frames[bone_index * animation.num_frames + i].timestamp;
		
		float left = timestamp - t;

		if (left <= 0)
		{
			frame_index = i;
			break;
		}
	}
	Frame* frame = &animation.frames[bone_index * animation.num_frames + frame_index];

	glm::mat4 local_transform = get_frame_matrix(*frame);

	Bone& bone = animation.bones[bone_index];

	animation.animation_transforms[bone_index] = parent_transform * local_transform;

	for (int i = 0; i < bone.num_children; i++)
	{
		calc_animation(animation, timestamp, bone.children[i], animation.animation_transforms[bone_index]);
	}

	animation.animation_transforms[bone_index] = animation.bones[bone_index].inv_bind_mat * animation.animation_transforms[bone_index];
}

void update_animation(AnimatedMesh& animation, float timestamp)
{
	for (int i = 0; i < animation.num_bones; i++)
	{
		animation.animation_transforms[i] = glm::mat4(1.0f);
	}
	calc_animation(animation, timestamp, 0, glm::mat4(1.0f));
}