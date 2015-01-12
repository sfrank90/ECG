#ifndef __MESH_OBJ__
#define __MESH_OBJ__

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <vector>
#include <stack>

#include <glm/glm.hpp>

// NEW: texture stuff //
// - Container for texture data
struct TextureData {
  TextureData() : data(NULL), width(0), height(0) {};
  unsigned char *data;
  unsigned int width;
  unsigned int height;
  // - OpenGL texture handle
  GLuint texture;
};

struct MeshMaterial {
	std::string name;
	glm::vec3 ambient_color;
	glm::vec3 diffuse_color;
	glm::vec3 specular_color;
	float specular_shininess;
	std::string texture_map;
};

struct MeshData {
  // data vectors //
  std::vector<GLfloat> vertex_position;
  std::vector<GLfloat> vertex_normal;
  std::vector<GLfloat> vertex_texcoord;
  // index list //
  std::vector<GLuint> indices;
};

class MeshObj {
  public:
    MeshObj();
    ~MeshObj();
    
	std::vector<MeshMaterial>& materials() {
		return m_materials;
	}
    void setData(const MeshData &data);
    void render(void);

	// - method to initialize the texture object
	void initTextures();
	// - method to load a texture from a given file
	TextureData loadTextureData(const char *fileName);
    
  private:
	  std::vector<MeshMaterial> m_materials;
    GLuint mVAO;
    GLuint mVBO_position;
    GLuint mVBO_normal;
    GLuint mVBO_texcoord;
    GLuint mIBO;
    GLuint mIndexCount;
};

#endif
