#pragma once

#include "Frost/Utils/Math/Vector.h"

#include <limits>

namespace Frost::Math
{
    struct Ray
    {
        Vector3 origin;
        Vector3 direction;
    };

    inline bool IntersectRayCylinder(const Ray& ray,
                                     const Vector3& cylinderBase,
                                     const Vector3& cylinderAxis,
                                     float cylinderRadius,
                                     float cylinderHeight,
                                     float& t)
    {
        Vector3 oc = ray.origin - cylinderBase;

        float card = Dot(cylinderAxis, ray.direction);
        float caoc = Dot(cylinderAxis, oc);

        float a = 1.0f - card * card;
        float b = Dot(oc, ray.direction) - caoc * card;
        float c = Dot(oc, oc) - caoc * caoc - cylinderRadius * cylinderRadius;

        float h = b * b - a * c;
        if (h < 0.0f)
            return false;

        h = sqrt(h);
        float t_intersect = (-b - h) / a;

        float y = caoc + t_intersect * card;
        if (y >= 0.0f && y <= cylinderHeight)
        {
            if (t_intersect > 0.0f)
            {
                t = t_intersect;
                return true;
            }
        }

        return false;
    }

    inline bool IntersectRayPlane(const Ray& ray, const Vector3& planePoint, const Vector3& planeNormal, float& t)
    {
        float denominator = Dot(planeNormal, ray.direction);
        if (std::abs(denominator) > 1e-6f)
        {
            Vector3 p0_l0 = planePoint - ray.origin;
            t = Dot(p0_l0, planeNormal) / denominator;
            return (t >= 0.0f);
        }
        return false;
    }

    inline bool IntersectRayQuad(const Ray& ray,
                                 const Vector3& p0,
                                 const Vector3& p1,
                                 const Vector3& p2,
                                 const Vector3& p3,
                                 float& t)
    {
        Vector3 edge1 = p1 - p0;
        Vector3 edge2 = p2 - p0;
        Vector3 h = Cross(ray.direction, edge2);
        float a = Dot(edge1, h);

        if (a > -1e-6f && a < 1e-6f)
            return false;

        float f = 1.0f / a;
        Vector3 s = ray.origin - p0;
        float u = f * Dot(s, h);

        if (u < 0.0f || u > 1.0f)
            return false;

        Vector3 q = Cross(s, edge1);
        float v = f * Dot(ray.direction, q);

        if (v < 0.0f || u + v > 1.0f)
        {
            edge1 = p2 - p0;
            edge2 = p3 - p0;
            h = Cross(ray.direction, edge2);
            a = Dot(edge1, h);

            if (a > -1e-6f && a < 1e-6f)
                return false;

            f = 1.0f / a;
            s = ray.origin - p0;
            u = f * Dot(s, h);

            if (u < 0.0f || u > 1.0f)
                return false;

            q = Cross(s, edge1);
            v = f * Dot(ray.direction, q);

            if (v < 0.0f || u + v > 1.0f)
                return false;
        }

        t = f * Dot(edge2, q);
        return t > 1e-6f;
    }

    inline bool IntersectRayTorus(const Ray& ray,
                                  const Vector3& torusCenter,
                                  const Vector3& torusNormal,
                                  float torusRadius,
                                  float torusThickness,
                                  float& t)
    {
        if (!IntersectRayPlane(ray, torusCenter, torusNormal, t))
        {
            return false;
        }

        Vector3 intersectionPoint = ray.origin + ray.direction * t;
        float distanceToCenter = Length(intersectionPoint - torusCenter);

        return (distanceToCenter >= torusRadius - torusThickness && distanceToCenter <= torusRadius + torusThickness);
    }

    inline bool IntersectRaySphere(const Ray& ray, const Vector3& sphereCenter, float sphereRadius, float& t)
    {
        Vector3 oc = ray.origin - sphereCenter;
        float b = Dot(oc, ray.direction);
        float c = Dot(oc, oc) - sphereRadius * sphereRadius;
        float h = b * b - c;

        // No intersection
        if (h < 0.0f)
            return false;

        h = sqrt(h);
        t = -b - h;

        // Camera inside the sphere
        if (t < 0.0f)
            t = -b + h;

        return t > 0.0f;
    }
} // namespace Frost::Math