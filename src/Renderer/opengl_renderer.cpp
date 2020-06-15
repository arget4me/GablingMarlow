#include "opengl_renderer.h"
#include <Utils/readfile.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "globals.h"

void checkShaderCompileError(GLint shaderID)
{
	GLint isCompiled = 0;
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &isCompiled);

	if (isCompiled == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		std::string errorLog;
		errorLog.resize(maxLength);
		glGetShaderInfoLog(shaderID, maxLength, &maxLength, &errorLog[0]);

		std::cout << "shader compilation failed:" << std::endl;
		std::cout << errorLog << std::endl;
		return;
	}
	else
		std::cout << "shader compilation success." << std::endl;

	return;
}

void loadShader(ShaderProgram & shaderprogram)
{
	//Read vertex sourcefile
	int vertex_filesize = 0;
	get_filesize(shaderprogram.vertex_source_path, &vertex_filesize);
	if (vertex_filesize == -1)
		return;
	char* vertex_shader_src = new char[vertex_filesize + 1];
	vertex_shader_src[vertex_filesize] = '\0';
	read_buffer(shaderprogram.vertex_source_path, vertex_shader_src, vertex_filesize);

	//Load fragment sourcefile
	int fragment_filesize = 0;
	get_filesize(shaderprogram.fragment_source_path, &fragment_filesize);
	if (fragment_filesize == -1)
		return;
	char* fragment_shader_src = new char[fragment_filesize + 1];
	fragment_shader_src[fragment_filesize] = '\0';
	read_buffer(shaderprogram.fragment_source_path, fragment_shader_src, fragment_filesize);

	//Comiple and link shader
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vertex_shader_src, NULL);
	glCompileShader(vs);
	checkShaderCompileError(vs);

	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fragment_shader_src, NULL);
	glCompileShader(fs);
	checkShaderCompileError(fs);

	shaderprogram.ID = glCreateProgram();
	glAttachShader(shaderprogram.ID, fs);
	glAttachShader(shaderprogram.ID, vs);
	glLinkProgram(shaderprogram.ID);

	//Cleanup
	glDeleteShader(vs);
	glDeleteShader(fs);
	delete[] vertex_shader_src;
	delete[] fragment_shader_src;
}


void setup_gl_renderer()
{
	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glClearColor(0.0f, 0.0f, 0.5f, 1.0f);
}


unsigned int get_next_mesh_id()
{
	static unsigned int id = 1;
	return id++;
}

Mesh upload_raw_mesh(RawMesh& raw_mesh)
{
	Mesh m;
	m.mesh_id = raw_mesh.mesh_id;
	m.index_count = raw_mesh.index_count;

	glGenVertexArrays(1, &m.mesh_vao);
	glGenBuffers(1, &m.mesh_vbo);
	glGenBuffers(1, &m.mesh_ebo);
	
	glBindVertexArray(m.mesh_vao);

	//upload vertices
	glBindBuffer(GL_ARRAY_BUFFER, m.mesh_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * raw_mesh.vertex_count, raw_mesh.vertex_buffer, GL_STATIC_DRAW);

	//upload indices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.mesh_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, raw_mesh.index_count * sizeof(unsigned int), raw_mesh.index_buffer, GL_STATIC_DRAW);

	//describe vertex layout
	//positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	
	//normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

	//uv coords
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));


	glBindVertexArray(0);

	return m;
}

void draw(Mesh m, ShaderProgram& shader, glm::vec3 model_origin, float time, float scale)
{
	//Do checks if already bound.


	glUseProgram(shader.ID);

	const float n = 1.0f;
	const float f = 1000.0f;
	glm::mat4 projectionMatrix = glm::perspective(glm::radians(45.0f), (float)global_width / (float)global_height, n, f);

	glm::mat4 modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, model_origin);
	modelMatrix = glm::rotate(modelMatrix, glm::radians(0.5f) * time, glm::vec3(0.2f, 0.3f, 0.5f));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(scale));

	glm::mat4 viewMatrix = glm::mat4(1.0f);
	viewMatrix = glm::translate(viewMatrix, glm::vec3(0.0f, 0.0f, 4.0f));
	glm::mat4 inverseViewMatrix = glm::inverse(viewMatrix);

	glm::mat4 modelViewMatrix = inverseViewMatrix * modelMatrix;

	glm::mat4 modelViewProjectionMatrix = projectionMatrix * modelViewMatrix;

	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "mvp"), 1, GL_FALSE, glm::value_ptr(modelViewProjectionMatrix));
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "mv"), 1, GL_FALSE, glm::value_ptr(modelMatrix));

	glBindVertexArray(m.mesh_vao);
	glDrawElements(GL_TRIANGLES, m.index_count, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}