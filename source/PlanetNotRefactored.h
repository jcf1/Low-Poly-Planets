/**
  \file Planet.h
http://blog.coredumping.com/subdivision-of-icosahedrons/
  */
#pragma once
#include <G3D/G3DAll.h>

/** \brief Application framework. */
class Planet{

protected:

    //Creates an initial icohedron with the given radius to be tessellated to create a sphere
    void makeIcohedron(float radius, Array<Vector3>& vertices, Array<Vector3int32>& faces);
    void subdivideIcoHedron(float radius, Array<Vector3>& vertices, Array<Vector3int32>& faces);
    void getMiddle(float radius, Vector3& v1, Vector3& v2, Vector3& newVector);

public:
    //Writes a sphere to a given off file
    void writeSphere(String filename, float radius, int depths, Array<Vector3>& vertices, Array<Vector3int32>& faces);
    void applyNoiseWater(Array<Vector3>& vertices, shared_ptr<Image> noise);
    void applyNoiseLand(Array<Vector3>& vertices, shared_ptr<Image> noise, shared_ptr<Image> test, float oceanLevel);
    void applyNoiseMountain(Array<Vector3>& vertices, shared_ptr<Image> noise, shared_ptr<Image> test, bool waterMount,  float power, float multiplier);
};
