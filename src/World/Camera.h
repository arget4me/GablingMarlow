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
	glm::mat4 inv_proj;
	float pitch;
	float yaw;
}Camera;

typedef struct{
	glm::vec3 origin;
	glm::vec3 direction;
}Ray;

Camera get_default_camera(float screen_width, float screen_height);
void recalculate_projection_matrix(Camera& camera, float screen_width, float screen_height);

inline glm::mat4 get_view_matrix(Camera& camera)
{
	return glm::lookAt(camera.position, camera.position + camera.dir, glm::vec3(0, 1, 0));
}

inline glm::mat4 get_inverse_view_matrix(Camera& camera)
{
	return glm::inverse(glm::lookAt(camera.position, camera.position + camera.dir, glm::vec3(0, 1, 0)));
}

/*
@breif Get ray from 2d screen position.
@param camera The camera used to cast ray
@param x_pos [-1, 1] where -1 is left of screen
@param y_pos [-1, 1] where -1 is bottom of screen
*/
Ray get_ray(Camera &camera, float x_pos, float y_pos);

void update_camera_orientation(Camera& camera, float delta_yaw, float delta_pitch, glm::vec3* follow = nullptr);

void update_camera(Camera &camera, glm::vec3* follow = nullptr);

#endif
