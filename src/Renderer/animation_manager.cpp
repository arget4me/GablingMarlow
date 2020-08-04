#include "animation_manager.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <limits>
#include <cmath>
#include <Utils/logfile.h>

inline Frame get_interpolated_frame(Frame& frame_0, Frame& frame_1, float blend_value)
{
	Frame interpolated_frame;
	interpolated_frame.position = (1.0f - blend_value) * frame_0.position + (blend_value)*frame_1.position;

	interpolated_frame.orientation = glm::mix(frame_0.orientation, frame_1.orientation, blend_value);

	return interpolated_frame;
}

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
	int frame_index_2 = 0;

	float closest_distance = timestamp;
	for (int i = 0; i < animation.num_frames; i++)
	{
		float t = animation.frames[bone_index * animation.num_frames + i].timestamp;
		
		float offset = fabs(timestamp - t);

		if (offset < closest_distance)
		{
			closest_distance = offset;
			frame_index = i;
			if (timestamp < t)
			{
				frame_index_2 = frame_index;
				frame_index = frame_index - 1;
				if (frame_index < 0)
				{
					frame_index = animation.num_frames - 1;
				}
			}
			else
			{
				frame_index = frame_index;
				frame_index_2 = (frame_index + 1) % animation.num_frames;
			}
		}
	}

	//Frame* frame = &animation.frames[bone_index * animation.num_frames + frame_index];
	//glm::mat4 local_transform = get_frame_matrix(*frame);
	Frame frame = {0};
	if (frame_index > frame_index_2)//Edge case looping
	{
		Frame& prev_frame = animation.frames[bone_index * animation.num_frames + frame_index];
		Frame& next_frame = animation.frames[bone_index * animation.num_frames + frame_index_2];
		float min = 0;
		float max = next_frame.timestamp;
		float frame_time = max - min;
		float delta_value = (timestamp - min) / frame_time;
		frame = get_interpolated_frame(prev_frame, next_frame, delta_value);
	}
	else
	{
		Frame& prev_frame = animation.frames[bone_index * animation.num_frames + frame_index];
		Frame& next_frame = animation.frames[bone_index * animation.num_frames + frame_index_2];
		float min = prev_frame.timestamp;
		float max = next_frame.timestamp;
		float frame_time = max - min;
		float delta_value = (timestamp - min) / frame_time;
		frame = get_interpolated_frame(prev_frame, next_frame, delta_value);
	}




	glm::mat4 local_transform = get_frame_matrix(frame);

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