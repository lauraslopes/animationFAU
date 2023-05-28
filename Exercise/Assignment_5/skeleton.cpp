#include "skeleton.h"
#include "geomutils.h"

Bone::Bone()
    : j0(-1),
      j1(-1),
      parent(-1),
      length(0)
{
    children.clear();
    t.setZero();
    R.setIdentity();
}

Bone::Bone(const Bone& other)
    : j0(other.j0),
      j1(other.j1),
      parent(other.parent),
      t(other.t),
      R(other.R),
      length(other.length)
{
    children = other.children;
}

void Bone::operator=(const Bone& other)
{
    j0 = other.j0;
    j1 = other.j1;
    parent = other.parent;
    children = other.children;
    t = other.t;
    R = other.R;
    length = other.length;
}

Skeleton::Skeleton()
{
    mJoints.clear();
    mBones.clear();
}

Skeleton::Skeleton(const Skeleton& other)
{
    mBones = other.mBones;
    mJoints = other.mJoints;
}

Skeleton::~Skeleton()
{
    mJoints.clear();
    mBones.clear();
}

unsigned int Skeleton::getNumJoints() const
{
    return mJoints.size();
}

unsigned int Skeleton::getNumBones() const
{
    return mBones.size();
}

void Skeleton::addJoint(const vec3 &pos)
{
    mJoints.push_back(pos);
}

bool Skeleton::getJoint(const unsigned int &idx, vec3 &out) const
{
	if(idx < mJoints.size())
	{
		out = mJoints[idx];
		return true;
	}

	return false;
}

bool Skeleton::setJoint(const unsigned int& idx, const vec3& _pos)
{
	if(idx >= mJoints.size())
	{
		PRINTERROR("Skeleton::setJoint error: out of bounds");
		return false;
	}

	mJoints[idx] = _pos;

	return true;
}

void Skeleton::addBone(int start, int end, int parent)
{
    if(((start == -1) || ((unsigned int) start < mJoints.size())) && ((unsigned int) end <= mJoints.size()))
    {
        Bone addB;
        addB.j0 = start;
        addB.j1 = end;
        addB.parent = parent; // parent bone
        if(parent > -1)
        {
            mBones[parent].children.push_back(mBones.size());
        }
        addB.length = (mJoints[start] - mJoints[end]).norm();
        mBones.push_back(addB);
        return;
    }
}

bool Skeleton::getBone(unsigned int idx, Bone& out) const
{
	if(idx < mBones.size())
	{
		out = mBones[idx];
		return true;
	}

	return false;
}

void Skeleton::getBoneLengths(std::vector<REAL> &out) const
{
	out.clear();
	for(unsigned int i = 0; i < mBones.size(); ++i)
	{
		out.push_back(mBones[i].length);
	}
}

bool Skeleton::getBoneRotationsAngles(const unsigned int& idx, vec3& angles) const
{
	if(idx >= mBones.size())
	{
		angles.setZero();
		return false;
	}

	// get a bones basis
	angles = mBones[idx].R.eulerAngles(0,1,2);

	return true;
}

bool Skeleton::setBoneRotationsAngles(const unsigned int& idx, const vec3& angles)
{
	if(idx >= mBones.size())
	{
		return false;
	}
	
	// rotation around x
	anax rotX(angles.x(), vec3::UnitX());	// pitch
	anax rotY(angles.y(), vec3::UnitY());	// yaw
	anax rotZ(angles.z(), vec3::UnitZ());	// roll
	quat q = rotX * rotY * rotZ;
	mBones[idx].R = q.matrix();

	std::vector<REAL> len;
	getBoneLengths(len);
	fitToBoneLengths(len);
	
	return true;
}

void Skeleton::updateBonesByJoints()
{
    for(unsigned int i = 0; i < mBones.size(); ++i)
    {
        if(mBones[i].j0 == -1 && mBones[i].j1 == -1)
        {
            PRINTERROR("invalid bone");
            continue;
        }

        if(mBones[i].j0 == -1)
        {
            mBones[i].t = mJoints[mBones[i].j1];
            mBones[i].R.setIdentity();
            continue;
        }

        if(mBones[i].j1 == -1)
        {
            mBones[i].t = mJoints[mBones[i].j0];
            mBones[i].R.setIdentity();
            continue;
        }

        mBones[i].t = mJoints[mBones[i].j0];
        mBones[i].length = (mJoints[mBones[i].j1] - mJoints[mBones[i].j0]).norm();

        // define an arbitrary basis aligned "up" to the bone direction
        // TODO: this basis should avoid to have two colinear vectors!!!!!!!
        vec3 d = mJoints[mBones[i].j1] - mJoints[mBones[i].j0];

        if(d.norm() == 0)
        {
            mBones[i].R.setIdentity();
            continue;
        }

        d.normalize();
        getGoodBasis(d, mBones[i].R);
    }
}

void Skeleton::fitToTargetSkeleton(const Skeleton &target)
{
    // find root bone
    int rootId = -1;
    for(unsigned int i = 0; i < mBones.size(); ++i)
    {
        if(mBones[i].parent == -1)
        {
            rootId = i;
            break;
        }
    }

    // is a root in the skeleton
    if(rootId == -1)
    {
        LOG("no root bone found");
        return;
    }

    // check for consistency
    if(mBones.size() != target.getNumBones())
    {
        LOG("skeletons does not match");
        return;
    }

    // init stack with root
    std::vector<int> ndStack(0);
    ndStack.push_back(rootId);

    // level pt
    int sp = 0;
    while((unsigned int)sp < ndStack.size())
    {
        Bone& cur = mBones[ndStack[sp]];

        for(unsigned int j = 0; j < cur.children.size(); ++j)
        {
            ndStack.push_back(cur.children[j]);
        }

        sp++;
    }

    // apply all rotations according stack order
    for(unsigned int k = 0; k < ndStack.size(); ++k)
    {
        // get current bone
        unsigned int l = ndStack[k];
        Bone& _tB = mBones[l];

        // transform rotation from other bone
        Bone _oB;
        target.getBone(l, _oB);
        _tB.R = _oB.R;

        // offset is the fathers end position
        if(_tB.parent > -1 && (unsigned int)_tB.parent < mBones.size())
        {
            Bone& _pP = mBones[_tB.parent];
            _tB.t = _pP.t + _pP.R.col(1) * _pP.length;
        }
        else
        {
            // LOG("no t correction for bone " << l);
        }

        mJoints[_tB.j0] = _tB.t;
        mJoints[_tB.j1] = _tB.t + _tB.R.col(1) * _tB.length;
    }
}

void Skeleton::fitToBoneLengths(const std::vector<REAL> &lengths)
{
    // find root bone
    int rootId = -1;
    for(unsigned int i = 0; i < mBones.size(); ++i)
    {
        if(mBones[i].parent == -1)
        {
            rootId = i;
            break;
        }
    }

    // is a root in the skeleton
    if(rootId == -1)
    {
        LOG("no root bone found");
        return;
    }

    // check for consistency
    if(mBones.size() != lengths.size())
    {
        LOG("skeletons does not match");
        return;
    }

    // init stack with root
    std::vector<int> ndStack(0);
    ndStack.push_back(rootId);

    // build hierarchy to stack
    int sp = 0;
    while((unsigned int)sp < ndStack.size())
    {
        Bone& cur = mBones[ndStack[sp]];

        for(unsigned int j = 0; j < cur.children.size(); ++j)
        {
            ndStack.push_back(cur.children[j]);
        }

        sp++;
    }

    // apply all length adjustments according stack order
    for(unsigned int k = 0; k < ndStack.size(); ++k)
    {
        // get current bone
        unsigned int l = ndStack[k];
        Bone& _tB = mBones[l];

        // apply length from given array 'lengths'
        _tB.length = lengths[l];

        // offset is the fathers end position
        if(_tB.parent > -1 && (unsigned int)_tB.parent < mBones.size())
        {
            Bone& _pP = mBones[_tB.parent];
            _tB.t = _pP.t + _pP.R.col(1) * _pP.length;
        }
        else
        {
            //LOG("no t correction for bone " << l);
        }

        mJoints[_tB.j0] = _tB.t;
        mJoints[_tB.j1] = _tB.t + _tB.R.col(1) * _tB.length;
    }
}


//!
MakeHSkeleton::MakeHSkeleton()
{
	mJoints.clear();
	mBones.clear();
	vec3 zero(0,0,0);
	addJoint(zero); //0: head
	addJoint(zero); //1: neck
	addJoint(zero); //2: left shoulder
	addJoint(zero); //3: right shoulder
	addJoint(zero); //4: torso
	addJoint(zero); //5: left elbow
	addJoint(zero); //6: right elbow
	addJoint(zero); //7: left hand
	addJoint(zero); //8: right hand
	addJoint(zero); //9: left hip
	addJoint(zero); //10: right hip
	addJoint(zero); //11: left knee
	addJoint(zero); //12: right knee
	addJoint(zero); //13: left foot
	addJoint(zero); //14: right foot
	addJoint(zero); //15: left finger
	addJoint(zero); //16: right finger
	addJoint(zero); //17: left toe
	addJoint(zero); //18: right toe

	addBone(0,1,-1); //0: head
	addBone(1,2,0); //1: left shoulder
	addBone(1,3,0); //2: right shoulder
	addBone(1,4,0); //3: breast
	addBone(4,9,3); //4: left abdomen
	addBone(4,10,3); //5: right abdomen
	addBone(2,5,1); //6: left upper arm
	addBone(3,6,2); //7: right upper arm
	addBone(9,11,4); //8: left upper leg
	addBone(10,12,5); //9: right upper leg
	addBone(5,7,6); //10: left lower arm
	addBone(6,8,7); //11: right lower arm
	addBone(11,13,8); //12: left lower leg
	addBone(12,14,9); //13: right lower leg
	addBone(7,15,10); //14: left hand
	addBone(8,16,11); //15: right hand
	addBone(13,17,12); //16: left foot
	addBone(14,18,13); //17: right foot
}

MakeHSkeleton::MakeHSkeleton(const MakeHSkeleton& other)
{
	mBones = other.mBones;
	mJoints = other.mJoints;
}

void MakeHSkeleton::fitToMakeHMesh(const std::vector<vec3>& _V)
{
	if(_V.size() != 19811)
	{
		PRINTERROR("no make human topology used");
		return;
	}

	for(unsigned int i = 0; i < 19; ++i)
	{
		vec3 jPos = REAL(0.5) * (_V[skelIdsMakeH[i][0]] + _V[skelIdsMakeH[i][1]]);
		setJoint(i, jPos);
	}
	
	updateBonesByJoints(_V);
}

void MakeHSkeleton::updateBonesByJoints()
{
	// set offset and calc lengths
	for(unsigned int i = 0; i < mBones.size(); ++i)
	{
		mBones[i].t = mJoints[mBones[i].j0];
		mBones[i].length = (mJoints[mBones[i].j1] - mJoints[mBones[i].j0]).norm();
	}
	
	// breast
	vec3 up = -(mJoints[4] - mJoints[1]).normalized();
	vec3 front = (mJoints[9]-mJoints[4]).cross(mJoints[10]-mJoints[4]);
	front.normalize();
	vec3 side = front.cross(up);
	side.normalize();
	mBones[3].R.col(0) = front;
	mBones[3].R.col(1) = up;
	mBones[3].R.col(2) = side;

	//left shoulder
	up = (mJoints[2] - mJoints[1]).normalized();
	front = up.cross(mJoints[4] - mJoints[1]);
	front.normalize();
	side = front.cross(up);
	side.normalize();
	mBones[1].R.col(0) = front;
	mBones[1].R.col(1) = up;
	mBones[1].R.col(2) = side;

	//right shoulder
	up = (mJoints[3] - mJoints[1]).normalized();
	front = up.cross(mJoints[1] - mJoints[4]);
	front.normalize();
	side = front.cross(up);
	side.normalize();
	mBones[2].R.col(0) = front;
	mBones[2].R.col(1) = up;
	mBones[2].R.col(2) = side;

	// head
	up = (mJoints[1] - mJoints[0]).normalized();
	front = mBones[3].R.col(0);
	side = front.cross(up);
	side.normalize();
	front = up.cross(side);
	front.normalize();
	mBones[0].R.col(0) = front;
	mBones[0].R.col(1) = up;
	mBones[0].R.col(2) = side;

	// left abdomen
	up = (mJoints[9]-mJoints[4]).normalized();
	front = up.cross(mJoints[4]-mJoints[1]);
	front.normalize();
	side = front.cross(up);
	side.normalize();
	mBones[4].R.col(0) = front;
	mBones[4].R.col(1) = up;
	mBones[4].R.col(2) = side;

	// right abdomen
	up = (mJoints[10]-mJoints[4]).normalized();
	front = up.cross(mJoints[1]-mJoints[4]);
	front.normalize();
	side = front.cross(up);
	side.normalize();
	mBones[5].R.col(0) = front;
	mBones[5].R.col(1) = up;
	mBones[5].R.col(2) = side;

	// left upper arm
	up = (mJoints[5]-mJoints[2]).normalized();
	front = up.cross(mJoints[4]-mJoints[1]);
	front.normalize();
	side = front.cross(up);
	side.normalize();
	mBones[6].R.col(0) = front;
	mBones[6].R.col(1) = up;
	mBones[6].R.col(2) = side;

	// right upper arm
	up = (mJoints[6]-mJoints[3]).normalized();
	front = up.cross(mJoints[1]-mJoints[4]);
	front.normalize();
	side = front.cross(up);
	side.normalize();
	mBones[7].R.col(0) = front;
	mBones[7].R.col(1) = up;
	mBones[7].R.col(2) = side;

	// left upper leg
	up = (mJoints[11]-mJoints[9]).normalized(); // knee - lhip
	front = up.cross(mJoints[10]-mJoints[9]); // rhip - lhip
	front.normalize();
	side = front.cross(up);
	side.normalize();
	mBones[8].R.col(0) = front;
	mBones[8].R.col(1) = up;
	mBones[8].R.col(2) = side;

	// right upper leg
	up = (mJoints[12]-mJoints[10]).normalized();
	front = up.cross(mJoints[10]-mJoints[9]);
	front.normalize();
	side = front.cross(up);
	side.normalize();
	mBones[9].R.col(0) = front;
	mBones[9].R.col(1) = up;
	mBones[9].R.col(2) = side;

	vec3 tmp;

	// left lower arm
	up = (mJoints[7]-mJoints[5]).normalized();      //lhand - lellbow
	tmp = (mJoints[5]-mJoints[2]).normalized();     //lellbow - lshoulder
	if(up.dot(tmp) > 0.1)
	{
		front = mBones[6].R.col(0);
		tmp = up.cross(front);
	}
	front = tmp.cross(up);
	front.normalize();
	side = front.cross(up);
	side.normalize();
	mBones[10].R.col(0) = front;
	mBones[10].R.col(1) = up;
	mBones[10].R.col(2) = side;


	// right lower arm
	up = (mJoints[8]-mJoints[6]).normalized();  // rhand - rellbow
	tmp = (mJoints[6]-mJoints[3]).normalized(); // rellbow - rshoulder
	if(up.dot(tmp) > 0.1)
	{
		front = mBones[7].R.col(0);
		tmp = up.cross(front);
	}
	front = tmp.cross(up);
	front.normalize();
	side = front.cross(up);
	side.normalize();
	mBones[11].R.col(0) = front;
	mBones[11].R.col(1) = up;
	mBones[11].R.col(2) = side;

	// left lower leg
	up = (mJoints[13]-mJoints[11]).normalized();    //lfoot - lknee
	tmp = (mJoints[11]-mJoints[12]).normalized();    // lknee - rknee
	if(up.dot(tmp) > 0.1)
	{
		front = mBones[8].R.col(0);
		tmp = up.cross(front);
	}
	front = tmp.cross(up);
	front.normalize();
	side = front.cross(up);
	side.normalize();
	mBones[12].R.col(0) = front;
	mBones[12].R.col(1) = up;
	mBones[12].R.col(2) = side;

	// right lower leg
	up = (mJoints[14]-mJoints[12]).normalized();
	tmp = (mJoints[11]-mJoints[12]).normalized(); // lknee - rknee
	if(up.dot(tmp) > 0.1)
	{
		front = mBones[9].R.col(0);
		tmp = up.cross(front);
	}
	front = tmp.cross(up);
	front.normalize();
	side = front.cross(up);
	side.normalize();
	mBones[13].R.col(0) = front;
	mBones[13].R.col(1) = up;
	mBones[13].R.col(2) = side;

	// left hand
	up = (mJoints[15] - mJoints[7]).normalized();
	front = mBones[10].R.col(0);
	side = front.cross(up);
	side.normalize();
	front = up.cross(side);
	front.normalize();
	mBones[14].R.col(0) = front;
	mBones[14].R.col(1) = up;
	mBones[14].R.col(2) = side;

	// right hand
	up = (mJoints[16] - mJoints[8]).normalized();
	front = mBones[11].R.col(0);
	side = front.cross(up);
	side.normalize();
	front = up.cross(side);
	front.normalize();
	mBones[15].R.col(0) = front;
	mBones[15].R.col(1) = up;
	mBones[15].R.col(2) = side;

	// left foot
	up = (mJoints[17] - mJoints[13]).normalized();  // ltoe - lfoot
	front = -mBones[12].R.col(1);
	side = up.cross(front).normalized();
	front = up.cross(side).normalized();
	mBones[16].R.col(0) = front;
	mBones[16].R.col(1) = up;
	mBones[16].R.col(2) = side;

	// right foot
	up = (mJoints[18] - mJoints[14]).normalized();
	front = -mBones[13].R.col(1);
	side = up.cross(front).normalized();
	front = up.cross(side).normalized();
	mBones[17].R.col(0) = front;
	mBones[17].R.col(1) = up;
	mBones[17].R.col(2) = side;
}

void MakeHSkeleton::updateBonesByJoints(const std::vector<vec3>& _V)
{
	if(_V.size() != 19811)
	{
		PRINTERROR("no make human topology used");
		return;
	}

	// set offset and calc lengths
	for(unsigned int i = 0; i < mBones.size(); ++i)
	{
		mBones[i].t = mJoints[mBones[i].j0];
		mBones[i].length = (mJoints[mBones[i].j1] - mJoints[mBones[i].j0]).norm();
	}

	// head
	vec3 up = (mJoints[1] - mJoints[0]).normalized();
	vec3 front = (_V[4434] - _V[4870]).normalized();
	vec3 side = up.cross(front).normalized();
	front = up.cross(side).normalized();
	mBones[0].R.col(0) = front;
	mBones[0].R.col(1) = up;
	mBones[0].R.col(2) = side;

	// left shoulder
	up = (mJoints[2] - mJoints[1]).normalized();
	front = (_V[12099] - _V[11909]).normalized();
	side = up.cross(front).normalized();
	front = up.cross(side).normalized();
	mBones[1].R.col(0) = front;
	mBones[1].R.col(1) = up;
	mBones[1].R.col(2) = side;
	
	// right shoulder
	up = (mJoints[3] - mJoints[1]).normalized();
	front = (_V[2207] - _V[2018]).normalized();
	side = up.cross(front).normalized();
	front = up.cross(side).normalized();
	mBones[2].R.col(0) = front;
	mBones[2].R.col(1) = up;
	mBones[2].R.col(2) = side;

	// breast
	up = (mJoints[4] - mJoints[1]).normalized();
	front = (_V[15414] - _V[5226]).normalized();
	side = up.cross(front).normalized();
	front = up.cross(side).normalized();
	mBones[3].R.col(0) = front;
	mBones[3].R.col(1) = up;
	mBones[3].R.col(2) = side;

	// left abdomen
	up = (mJoints[9] - mJoints[4]).normalized();
	front = (_V[12666] - _V[10996]).normalized();
	side = up.cross(front).normalized();
	front = up.cross(side).normalized();
	mBones[4].R.col(0) = front;
	mBones[4].R.col(1) = up;
	mBones[4].R.col(2) = side;

	// right abdomen
	up = (mJoints[10] - mJoints[4]).normalized();
	front = (_V[2775] - _V[1103]).normalized();
	side = up.cross(front).normalized();
	front = up.cross(side).normalized();
	mBones[5].R.col(0) = front;
	mBones[5].R.col(1) = up;
	mBones[5].R.col(2) = side;
	
	// left upper arm
	up = (mJoints[5] - mJoints[2]).normalized();
	front = (_V[11550] - _V[15218]).normalized();
	side = up.cross(front).normalized();
	front = up.cross(side).normalized();
	mBones[6].R.col(0) = front;
	mBones[6].R.col(1) = up;
	mBones[6].R.col(2) = side;

	// right upper arm
	up = (mJoints[6] - mJoints[3]).normalized();
	front = (_V[1659] - _V[5323]).normalized();
	side = up.cross(front).normalized();
	front = up.cross(side).normalized();
	mBones[7].R.col(0) = front;
	mBones[7].R.col(1) = up;
	mBones[7].R.col(2) = side;

	// left upper leg
	up = (mJoints[11] - mJoints[9]).normalized();
	front = (_V[13361] - _V[10389]).normalized();
	side = up.cross(front).normalized();
	front = up.cross(side).normalized();
	mBones[8].R.col(0) = front;
	mBones[8].R.col(1) = up;
	mBones[8].R.col(2) = side;
	
	// right upper leg
	up = (mJoints[12] - mJoints[10]).normalized();
	front = (_V[3466] - _V[496]).normalized();
	side = up.cross(front).normalized();
	front = up.cross(side).normalized();
	mBones[9].R.col(0) = front;
	mBones[9].R.col(1) = up;
	mBones[9].R.col(2) = side;

	// left lower arm
	up = (mJoints[7] - mJoints[5]).normalized();
	front = (_V[9908] - _V[16966]).normalized();
	side = up.cross(front).normalized();
	front = up.cross(side).normalized();
	mBones[10].R.col(0) = front;
	mBones[10].R.col(1) = up;
	mBones[10].R.col(2) = side;

	// right lower arm
	up = (mJoints[8] - mJoints[6]).normalized();
	front = (_V[13] - _V[7073]).normalized();
	side = up.cross(front).normalized();
	front = up.cross(side).normalized();
	mBones[11].R.col(0) = front;
	mBones[11].R.col(1) = up;
	mBones[11].R.col(2) = side;

	// left lower leg
	up = (mJoints[13] - mJoints[11]).normalized();
	front = (_V[18182] - _V[11443]).normalized();
	side = up.cross(front).normalized();
	front = up.cross(side).normalized();
	mBones[12].R.col(0) = front;
	mBones[12].R.col(1) = up;
	mBones[12].R.col(2) = side;

	// right lower leg
	up = (mJoints[14] - mJoints[12]).normalized();
	front = (_V[8267] - _V[1551]).normalized();
	side = up.cross(front).normalized();
	front = up.cross(side).normalized();
	mBones[13].R.col(0) = front;
	mBones[13].R.col(1) = up;
	mBones[13].R.col(2) = side;

	// left hand
	up = (mJoints[15] - mJoints[7]).normalized();
	front = (_V[17358] - _V[16755]).normalized();
	side = up.cross(front).normalized();
	front = up.cross(side).normalized();
	mBones[14].R.col(0) = front;
	mBones[14].R.col(1) = up;
	mBones[14].R.col(2) = side;

	// right hand
	up = (mJoints[16] - mJoints[8]).normalized();
	front = (_V[7464] - _V[6862]).normalized();
	side = up.cross(front).normalized();
	front = up.cross(side).normalized();
	mBones[15].R.col(0) = front;
	mBones[15].R.col(1) = up;
	mBones[15].R.col(2) = side;

	// left foot
	up = (mJoints[17] - mJoints[13]).normalized();
	front = (_V[19263] - _V[10545]).normalized();
	side = up.cross(front).normalized();
	front = up.cross(side).normalized();
	mBones[16].R.col(0) = front;
	mBones[16].R.col(1) = up;
	mBones[16].R.col(2) = side;

	// right foot
	up = (mJoints[18] - mJoints[14]).normalized();
	front = (_V[9346] - _V[652]).normalized();
	side = up.cross(front).normalized();
	front = up.cross(side).normalized();
	mBones[17].R.col(0) = front;
	mBones[17].R.col(1) = up;
	mBones[17].R.col(2) = side;
}

void MakeHSkeleton::operator=(const MakeHSkeleton& other)
{
	mBones = other.mBones;
	mJoints = other.mJoints;
}
