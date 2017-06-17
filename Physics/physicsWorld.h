#pragma once

#include <vector>
#include <Base.h>

#include <physicsBody.h> /// For physicsMotionType
#include <physicsShape.h> /// For physicsShape::NUM_SHAPES
#include <physicsCollider.h>
#include <physicsSolver.h>

struct ContactPoint;
class physicsSolver;

typedef void( *ColliderFuncPtr )( const std::shared_ptr<physicsShape>& shapeA, 
								  const std::shared_ptr<physicsShape>& shapeB,
								  const Matrix3& transformA,
								  const Matrix3& transformB,
								  std::vector<ContactPoint>& contacts);

struct physicsWorldCinfo
{
	Vector3 m_gravity;
	Real m_deltaTime;
	Real m_cor;
	int m_numIter;

	physicsWorldCinfo() :
		m_gravity( 0.f, -98.f ),
		m_deltaTime( .016f ),
		m_cor( 1.f ),
		m_numIter( 8 ) {}
};

struct JointCinfo
{
	int bodyIdA;
	int bodyIdB;
	Vector3 pivot;
};

struct CachedPair : public BodyIdPair
{
	ContactPoint cpA;
	ContactPoint cpB;
	Real accumImp;
	int numContacts;

private:

	

public:

	CachedPair( const BodyId a, const BodyId b ):
		BodyIdPair( a, b ),
		cpA(), cpB(), accumImp(0.f), numContacts( 0 )
	{

	}

	CachedPair( const BodyIdPair& other ) :
		BodyIdPair( other ),
		cpA(), cpB(), accumImp(0.f), numContacts( 0 )
	{

	}

	/// TODO: move implementation out
	void addContact(const ContactPoint cp)
	{
		if ( numContacts == 0 )
		{
			cpA = cp;
			numContacts = 1;
		}
		else if ( numContacts == 1 )
		{
			Real distToA;
			ContactPointUtils::getContactDifference( cp, cpA, distToA );
			if ( distToA < 5.f )
			{
				cpA = cp;
				numContacts = 1;
			}
			else
			{
				cpB = cp;
				numContacts = 2;
			}
		}
		else
		{
			Real distToA, distToB;
			ContactPointUtils::getContactDifference( cp, cpA, distToA );
			ContactPointUtils::getContactDifference( cp, cpB, distToB );

			if ( distToA < 5.f )
			{
				cpA = cp;
				numContacts = 1;
			}
			else if ( distToB < 5.f )
			{
				cpA = cp;
				numContacts = 1;
			}
			( distToA < distToB ) ? cpA = cp : cpB = cp;
		}
	}
};

class physicsWorld : public physicsObject
{
public:

	physicsWorld( const physicsWorldCinfo& cinfo );

	~physicsWorld();

	BodyId createBody( const physicsBodyCinfo& cinfo );

	void removeBody( const BodyId bodyId );

	inline const std::vector<BodyId>& getActiveBodyIds() const;

	const physicsBody& getBody( const BodyId bodyId ) const;

	int addJoint( const JointCinfo& config );

	void removeJoint( JointId jointId );

	void step();
	
	void render(); /// TODO: move this out of API

	/// Utility funcs
	void setPosition( BodyId bodyId, const Vector3& point );

	void setMotionType( BodyId bodyId, physicsMotionType type );

	const Real getDeltaTime();

protected:

	Vector3 m_gravity;
	Real m_cor;
	std::vector<physicsBody> m_bodies; /// Simulated, free bodies
	std::vector<struct BroadphaseBody> m_broadphaseBodies;
	SolverInfo m_solverInfo;
	physicsSolver* m_solver;
	ColliderFuncPtr m_dispatchTable[physicsShape::NUM_SHAPES][physicsShape::NUM_SHAPES];
	std::vector<BodyIdPair> m_newPairs; /// New broadphase pairs
	std::vector<BodyIdPair> m_existingPairs; /// Existing broadphase pairs
	std::vector<CachedPair> m_cachedPairs;
	std::vector<ConstrainedPair> m_jointSolvePairs;
	std::vector<ConstrainedPair> m_contactSolvePairs;
	std::vector<BodyId> m_activeBodyIds; /// Simulated body Ids
	std::vector<SolverBody> m_solverBodies;
	BodyId m_firstFreeBodyId;
};

#include <physicsWorld.inl>