#include "MeshObj.h"
#include <iostream>
#include <limits>

MeshObj::MeshObj() {
  mVAO = 0;
  mVBO_position = 0;
  mVBO_normal = 0;
  // HINT : texture coords are not used yet
  mIBO = 0;
  mIndexCount = 0;
}

MeshObj::~MeshObj() {
  glDeleteBuffers(1, &mIBO);
  glDeleteBuffers(1, &mVBO_position);
  glDeleteBuffers(1, &mVBO_normal);
  // HINT : texture coords are not used yet

  glDeleteVertexArrays(1, &mVAO);
}

void MeshObj::setData(const MeshData &meshData) {
  mIndexCount = meshData.indices.size();

  GLuint* indizes = new GLuint[mIndexCount];
  for (int i = 0; i < mIndexCount; i++)
	  indizes[i] = (GLuint)meshData.indices[i];
  
  // HINT : texture coords are not used yet for rendering

  // TODO: create local storage arrays for vertices, normals and indices //
  int size = meshData.vertex_position.size();
  GLfloat* verticesArray = new GLfloat[size];
  GLfloat* normalsArray = new GLfloat[size];

  // TODO: copy data into local arrays //
  for (int i = 0; i < size; i++)
  {
	  verticesArray[i] = meshData.vertex_position[i];
	  normalsArray[i] = meshData.vertex_normal[i];
  }
  
  // TODO: create VAO //
  glGenVertexArrays(1, &mVAO);
  glBindVertexArray(mVAO);
  
  // TODO: create and bind VBOs and upload data (one VBO per vertex attribute -> position, normal) //
  glGenBuffers(1, &mVBO_position);
  glBindBuffer(GL_ARRAY_BUFFER, mVBO_position);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*size, verticesArray, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
  glEnableVertexAttribArray(0);

  glGenBuffers(1, &mVBO_normal);
  glBindBuffer(GL_ARRAY_BUFFER, mVBO_normal);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*size, normalsArray, GL_STATIC_DRAW);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
  glEnableVertexAttribArray(1);

  // TODO: init and bind a IBO //
  glGenBuffers(1, &mIBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndexCount * sizeof(GLuint), indizes, GL_STATIC_DRAW);

  // unbind buffers //
  glBindVertexArray(0);
  
  // TODO: make sure to clean up temporarily allocated data, if neccessary //
  delete[] verticesArray;
  delete[] normalsArray;
  delete[] indizes;
}

void MeshObj::render(void) {
  if (mVAO != 0) {
    // TODO: render your VAO //
	  glBindVertexArray(mVAO);

	  glDrawElements(GL_TRIANGLES, mIndexCount, GL_UNSIGNED_INT, (void*)0);
    
    glBindVertexArray(0);
  }
  if (mVAO != 0) {
	  // TODO: render your VAO //

	  glBindVertexArray(mVAO);

	  // Draw the triangles !
	  glDrawElements(GL_TRIANGLES, mIndexCount, GL_UNSIGNED_INT, (void*)0);

	  glBindVertexArray(0);
  }
  else
  {
	  std::cout << "mVao ist not correct" << std::endl;
	  exit(-1);
  }
}
