/** \file SimpleMesh.cpp */
#include "SimpleMesh.h"

SimpleMesh::SimpleMesh(const Array<Vector3>& vertexPositions, const Array<Vector3int32>& triArray, Array<Vector3> normals, Array<Vector2> texture){
    MeshBuilder builder;
    for (int i = 0; i < triArray.size(); ++i) {
        Vector3int32 v = triArray[i];
        m_indexArray.append(v.x, v.y, v.z);
    }
    m_vertexPositions = vertexPositions;
    m_triArray = triArray;
    m_normals = normals;
    m_texture = texture;
}

void SimpleMesh::toObj(String filename){

    TextOutput file(filename);
    
    for (int i = 0; i < m_vertexPositions.size(); ++i) {
        file.printf(STR(v %f %f %f\n), m_vertexPositions[i].x, m_vertexPositions[i].y, m_vertexPositions[i].z);
    }
    /*
    /*for (int i = 0; i < m_texture.size(); ++i) {
        file.printf(STR(vt %f %f 0\n), m_texture[i].x, m_texture[i].y);
    }

    for (int i = 0; i < m_normals.size(); ++i) {
        file.printf(STR(vn %f %f %f\n), m_normals[i].x, m_normals[i].y, m_normals[i].z);
    }*/
    //using m_indexArray
    for (int i = 0; i < m_indexArray.size(); i+=3) {
        file.printf(STR(f %d %d %d \n), m_indexArray[i]+1, m_indexArray[i+1]+1, m_indexArray[i+2]+1);
        //file.printf(STR(f %d/%d/%d %d/%d/%d %d/%d/%d \n), m_indexArray[i]+1,m_indexArray[i]+1,m_indexArray[i]+1, m_indexArray[i+1]+1, m_indexArray[i+1]+1, m_indexArray[i+1]+1, m_indexArray[i+2]+1,  m_indexArray[i+2]+1,  m_indexArray[i+2]+1);
    }
    /*
    file.printf(STR(# vertices = %d), m_vertexPositions.size());

    file.printf(STR(# normals = %d), m_normals.size());

    //file.printf(STR(# texture = %d), m_texture.size());

    
    file.printf(STR(# faces = %d), m_triArray.size());
    */
    file.printf(STR(\n));
    file.commit();

}


    //file.printf("g name\n");
        //loop to make vertices
    //debugPrintf(STR(%d\n), sizeof(m_vertexArray));

    //Loop for faces
    //using m_triArray
    /*
    for (int i = 0; i < m_triArray.size(); ++i) {
        file.printf(STR(f %d %d %d\n), m_triArray[i].x, m_triArray[i].y, m_triArray[i].z);
    }*/

    /*TextOutput file(filename);
    //file.printf("g name\n");
        //loop to make vertices
    //debugPrintf(STR(%d\n), sizeof(m_vertexArray));
    for(int i = 0; i < m_vertexArray.size(); ++i){
        file.printf(STR(v %f %f %f\n), m_vertexArray[i].x, m_vertexArray[i].y, m_vertexArray[i].z);
    }

    //Loop for faces
    for(int i = 0; i < m_triArray.size(); ++i){
        file.printf(STR(f %d %d %d\n), m_triArray[i].x, m_triArray[i].y, m_triArray[i].z);
    }
    file.printf(STR(\n));
    file.commit();*/