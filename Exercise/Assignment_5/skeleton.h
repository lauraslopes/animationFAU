#ifndef __SKELETON_H__
#define __SKELETON_H__

#include "platform.h"

struct Bone
{
    //! the bones start joint id
    int j0;

    //! the bones end joint id
    int j1;

    //! the bone's parent bone
    int parent;

    //! the bone's children bones
    std::vector<int> children;

    //! the bones offset to world origin
    vec3 t;

    //! the bones local orthonormal basis
    mat3 R;

    //! the original bone length set once on skel fitting
    REAL length;

    //! constructor
    Bone();

    //! copy constructor
    Bone(const Bone& other);

    //! operator= overloaded to have assignment possible
    void operator=(const Bone& other);
};


class Skeleton
{

public:

    //! constructor
    Skeleton();

    //! copy constructor
    Skeleton(const Skeleton& other);

    //! destructor
    ~Skeleton();

    //! get number of joints
    unsigned int getNumJoints() const;

    //! get number of bones
    unsigned int getNumBones() const;

    //! add a new joint
    void addJoint(const vec3& pos);

	//! retrieve the joints coordinate
	bool getJoint(const unsigned int& idx, vec3& out) const;

	//! set the position of the joint denoted by idx
	bool setJoint(const unsigned int &idx, const vec3& _pos);

    //! add a new bone
    void addBone(int start, int end, int parent);
	
	//! get the handle to a bone
	bool getBone(unsigned int idx, Bone& out) const;

	//! get all bone lengths
	void getBoneLengths(std::vector<REAL>& out) const;

	//! get a bones euler angles
	bool getBoneRotationsAngles(const unsigned int& idx, vec3& angles) const;
	
	//! set a bone by euler angles
	bool setBoneRotationsAngles(const unsigned int& idx, const vec3& angles);
	    
    //! updates all bone's bases after joints (inverse kinematics)
    void updateBonesByJoints();

    //! fits a skeleton to match best the target - keep lengths
    void fitToTargetSkeleton(const Skeleton& target);

    //! fits a skeleton to match the given bone lengths - keep rotations
    void fitToBoneLengths(const std::vector<REAL>& lengths);
	
protected:

    //! joints describing the keys for the skeleton
    std::vector<vec3> mJoints;

    //! bones are the interconnections <from, to>
    std::vector<Bone> mBones;
};

class MakeHSkeleton : public Skeleton
{
public:

	//! constructor
	MakeHSkeleton();

	//! copyconstructor
	MakeHSkeleton(const MakeHSkeleton& other);

	//! given vertices from a make human mesh, fits the skeleton proper
	void fitToMakeHMesh(const std::vector<vec3>& _V);

	//! updates all bone's bases after joints (scape specific override)
	void updateBonesByJoints();

	//! updates all bone's bases after joints (scape specific override)
	void updateBonesByJoints(const std::vector<vec3>& _V);

	//! assignment operator
	void operator=(const MakeHSkeleton& other);

protected:

};

static unsigned int skelIdsMakeH[19][2] = 
{
	{8567, 4870}, // head
	{14675, 5137}, // neck
	{12618, 11324}, // l. shoulder
	{2728, 1432}, // r. shoulder
	{7952, 17867}, // torso
	{17390, 16872}, // l. ellbow
	{7496, 6979}, // r. ellbow
	{16772, 17174}, // l. hand
	{6879, 7281}, // r. hand
	{10478, 12366}, // l. hip
	{585, 2476}, // r. hip
	{13068, 18095}, // l. knee
	{3174, 8180}, // r. knee
	{19416, 18412}, // l. foot
	{9499, 8497}, // r. foot
	{15957, 15957}, // l. finger
	{6064, 6064}, // r. finger
	{19495, 19495}, // l. toe
	{9579, 9579} // r. toe
};

#endif //__SKELETON_H__
