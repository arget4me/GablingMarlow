#ifndef CAMERA_HEADER
#define CAMERA_HEADER


#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>

typedef struct {
	glm::vec3 position;
	glm::vec3 dir;
	glm::vec3 right;
	glm::mat4 proj;
	float pitch;
	float yaw;
}Camera;

Camera get_default_camera(float screen_width, float screen_height);
void recalculate_projection_matrix(Camera& camera, float screen_width, float screen_height);

inline glm::mat4 get_view_matrix(Camera& camera)
{
	return glm::lookAt(camera.position, camera.position + camera.dir, glm::vec3(0, 1, 0));
}

void update_camera_orientation(Camera& camera, float delta_yaw, float delta_pitch);

void update_camera(Camera &camera);

#endif
