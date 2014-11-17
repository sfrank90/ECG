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
  
  // HINT : texture coords are not used yet for rendering

  // TODO: create local storage arrays for vertices, normals and indices //
  
  // TODO: copy data into local arrays //
  
  // TODO: create VAO //
  
  // TODO: create and bind VBOs and upload data (one VBO per vertex attribute -> position, normal) //
  
  // TODO: init and bind a IBO //

  // unbind buffers //
  glBindVertexArray(0);
  
  // TODO: make sure to clean up temporarily allocated data, if neccessary //
}

void MeshObj::render(void) {
  if (mVAO != 0) {
    // TODO: render your VAO //
    
    glBindVertexArray(0);
  }
}
