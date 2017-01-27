/**
  \file SimpleMesh.h
   
   Defines mesh and its operators, including edge collapse and beveling.
 */
/*
    John Freeman 
    Jose Rivas-Garcia 
    Julia Goldman 
    Matheus de Carvalho Souza
*/
#pragma once
#include <G3D/G3DAll.h>

class SimpleMesh { 
protected:
    Array<Vector3> m_vertexPositions;
    Array<int> m_indexArray;
    Array<Vector3int32> m_triArray;
    Array<Vector3> m_normals;
    Array<Vector2> m_texture;

public:
    void toObj(String filename);
    SimpleMesh(const Array<Vector3>& vertexPositions, const Array<Vector3int32>& triArray, Array<Vector3> normals, Array<Vector2> texture);
    //~SimpleMesh();
};