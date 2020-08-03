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
	float max_timestamp = animation.frames[(bone_index + 1) * animation.num_frames - 1].timestamp;

	timestamp = fmod(timestamp, max_timestamp);
	int frame_index = animation.num_frames - 1;
	float closest_distance = timestamp;
	for (int i = 0; i < animation.num_frames; i++)
	{
		float t = animation.frames[bone_index * animation.num_frames + i].timestamp;
		
		float offset = fabs(timestamp - t);

		if (offset < closest_distance)
		{
			closest_distance = offset;
			frame_index = i;
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

	animation.animation_transforms[bone_index] = animation.animation_transforms[bone_index] * animation.bones[bone_index].inv_bind_mat;
}

void update_animation(AnimatedMesh& animation, float timestamp)
{
	calc_animation(animation, timestamp, 0, glm::mat4(1.0f));
}