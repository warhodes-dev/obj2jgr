#include <iostream>
#include <vector>
#include <string>
#include <cmath> //trig functions
using namespace std;

/* Mesh and Matrix data structures */

struct vec3d {
	float x = 0;
	float y = 0;
	float z = 0;
	float w = 1;
};

struct triangle {
	vec3d p[3];
	float color; //grayscale, 0-1
};

struct mesh {
	vector<triangle> tris;
};

struct mat4x4 {
	float m[4][4] = { 0 };
};



/* Matrix transformation helper functions */

void multiplyMatrix(vec3d &i, vec3d &o, mat4x4 &m)
{
	/* Multiplies i by m into output o */
	float w;
	o.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + m.m[3][0];
	o.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + m.m[3][1];
	o.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + m.m[3][2];
	  w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + m.m[3][3];
	
	if (w != 0.0f) {
		o.x /= w;
		o.y /= w;
		o.z /= w;
	}
}

mat4x4 makeTranslation(float x, float y, float z)
{
	mat4x4 matrix;
	matrix.m[0][0] = 1.0f;
	matrix.m[1][1] = 1.0f;
	matrix.m[2][2] = 1.0f;
	matrix.m[3][3] = 1.0f;
	matrix.m[3][0] = x;
	matrix.m[3][1] = y;
	matrix.m[3][2] = z;
	return matrix;
}

mat4x4 makeRotationX(float fTheta)
{
	mat4x4 matrix;
	fTheta = fTheta * (3.14159/180);
	matrix.m[0][0] = 1.0f;
	matrix.m[1][1] = cosf(fTheta);
	matrix.m[1][2] = sinf(fTheta);
	matrix.m[2][1] = -sinf(fTheta);
	matrix.m[2][2] = cosf(fTheta);
	matrix.m[3][3] = 1.0f;
	return matrix;
}

mat4x4 makeRotationY(float fTheta)
{
	mat4x4 matrix;
	fTheta = fTheta * (3.14159/180);
	matrix.m[0][0] = cosf(fTheta);
	matrix.m[0][2] = sinf(fTheta);
	matrix.m[2][0] = -sinf(fTheta);
	matrix.m[1][1] = 1.0f;
	matrix.m[2][2] = cosf(fTheta);
	matrix.m[3][3] = 1.0f;
	return matrix;
}

mat4x4 makeProjectionMatrix(float fFovDeg, float fAspectRatio, float fNear, float fFar)
{

	/* This is essntially the core of the entire program.
	 *
	 * An easy way to intuit what this projection matrix is doing is to
	 * think of each diagonal coordinate as acting on the x ([0][0]),
	 * y ([1][1]) and z ([2][2]) coordinates of every vertex that is being
	 * multiplied by the matrix. 
	 *
	 * First, the x coordinates must be multiplied by the aspect ratio of the
	 * screen being drawn to. In this case, we are exclusively drawing to a
	 * square jgraph screen, so this can be ignored (set to 1).
	 *
	 * Second, the x and y coordinates must be transformed by fFovRad, which is
	 * derived from fFovDeg and accounts for the fact that human vision is 
	 * conal rather than orthogonal. i.e. things that are farther away occupy 
	 * less of your field of vision than things right in front of your face.
	 *
	 * Finally, z coordinates must be scaled based on their proportion of
	 * how close they are in 3D space to a kind of virtual 'screen'. This
	 * doesn't correspond to an immediate visual effect, but is useful when
	 * drawing polygons as it helps determine the order of the polygon layering,
	 * which in this case means the order of the .jgr lines.
	 *
	 * Additionally notice that the matrix is 4x4 instead of3x3. This is to allow
	 * us to extract the pre-offset transformed z coordinates for use later when
	 * normalizing for z.
	 */

	float fFovRad = 1.0f / tanf(fFovDeg * 0.5f / 180.0f * 3.14159f);
	mat4x4 matrix;
	matrix.m[0][0] = fAspectRatio * fFovRad;
	matrix.m[1][1] = fFovRad;
	matrix.m[2][2] = fFar / (fFar - fNear);
	matrix.m[3][2] = (-fFar * fNear) / (fFar - fNear);
	matrix.m[2][3] = 1.0f;
	matrix.m[3][3] = 0.0f;
	return matrix;
}



/* Output helper functions */

void drawTriangle(float x1, float y1, 
				  float x2, float y2, 
				  float x3, float y3, 
				  float color, float fill)
{
	printf("newline poly linethickness 0 gray %f pfill %f pts %f %f  %f %f  %f %f\n\n",
			color, fill, x1, y1, x2, y2, x3, y3);
	return;
}

/* Main */

int main (int argc, char **argv)
{
	float fTheta;
	mesh Mesh;
	mat4x4 matProj, matRotX, matRotY;
	vec3d vCamera {0.0f, 0.0f, 0.0f};
	vec3d vLightSource {0.0f, 1.0f, -1.0f};
	float xrot, yrot, offset;

	/*** Step -1: Check the command line args ***/
	if (argc != 4) {
		printf("usage: obj2jgr   x rotation (deg)   y rotation (deg)   zoom\n \
					zoom should be set larger until the full model is visible.\n");
	} else {
		xrot = stof(argv[1]);
		yrot = stof(argv[2]);
		offset = stof(argv[3]);
	}

	/*** Step 0: Set up the jgr file ***/

	int screenwidth = 100;
	int screenheight = 100;

	printf("newgraph\n\n");
	printf("xaxis min 0 max 100 size 2 hash 0 nodraw\n");
	printf("yaxis min 0 max 100 size 2 hash 0 nodraw\n\n");
	printf("newline poly pfill 0 pts 0 0  100 0  100 100  0 100\n\n");

	/*** Step 1: Load 3D model file into Mesh from standard input ***
	 *
	 * Notice: Only supports .obj files with vertex and face data.
	 *		  Texture data is not supported.
	 * Explanation:
	 *		  .obj files contain lines of the format "s x y z" where
	 *		  s is a string representing the 'type' of the following
	 *		  x y z values, and x y z are either floats, ints, pairs
	 *		  of floats/ints, or triplets of floats/ints. In this
	 *		  program, I only read vertex data (of the form "v x y z")
	 *		  and face data (of the form "f x/xt/xn y/yt/yn z/zt/zn").
	 *		  So texture maps, flags, or other such things are omitted.
	 */

	string line;
	vector<vec3d> verts; //temporary cache of vertices
	while (getline(cin, line)) {

		if (line[0] == 'v' && line[1] == ' ') {
			vec3d v;
			sscanf(line.c_str(), "v %f %f %f\n", &v.x, &v.y, &v.z);

			//v.x /= 10; v.y /= 10; v.z /= 10;

			verts.push_back(v); //index of v is it's index in the .obj
		}
		
		if (line[0] == 'f') {
			int f[3];
			sscanf(line.c_str(), "f %d/%*d/%*d %d/%*d/%*d %d/%*d/%*d\n", 
					&f[0], &f[1], &f[2]);
			Mesh.tris.push_back({ verts[f[0] - 1], verts[f[1] - 1], verts[f[2] - 1] });
		}
	}

	/*** Step 2: Create projection and translation matrices ***
	 *
	 * See makeProjectionMatrix() above
	 *
	 * Explanation:
	 * 		A 4x4 matrix will be used to transform all vertices from their
	 * 		3D coordinates into a flattened 2D equivalent that adheres to
	 * 		the expected rules of perspective in order to give the illusion
	 * 		of a 3D image drawn on a 2D screen.
	 *
	 * 		In addition, other matrix operations such as rotation and
	 * 		translation can be used
	 */

	matProj = makeProjectionMatrix(90.0f, 1, 0.1f, 1000.0f);
	matRotX = makeRotationX(xrot);
	matRotY = makeRotationY(yrot);

	/*** Step 3: Transform and draw every triangle ***
	 *	
	 *	Every single triangle needs to undergo a series of transformations,
	 *	including the main projection matrix transformation.
	 *
	 *	After the triangles are projected properly, some additional
	 *	calculations are done to figure out which triangles to omit
	 *	(because they can't be seen) and what shade the remaining
	 *	triangles should be based on their normal angle to a light
	 *	source (otherwise you just have a wireframe).
	 */

	for (auto tri : Mesh.tris) {

		triangle triProjected, triTranslated, triRotatedX, triRotatedY;

		//Rotate the triangle in X
		multiplyMatrix(tri.p[0], triRotatedX.p[0], matRotX);
		multiplyMatrix(tri.p[1], triRotatedX.p[1], matRotX);
		multiplyMatrix(tri.p[2], triRotatedX.p[2], matRotX);

		//Rotate the triangle in Y
		multiplyMatrix(triRotatedX.p[0], triRotatedY.p[0], matRotY);
		multiplyMatrix(triRotatedX.p[1], triRotatedY.p[1], matRotY);
		multiplyMatrix(triRotatedX.p[2], triRotatedY.p[2], matRotY);

		//Translate the triangle into a more useful location
		triTranslated = triRotatedY;
		triTranslated.p[0].z = triRotatedY.p[0].z + offset;
		triTranslated.p[1].z = triRotatedY.p[1].z + offset;
		triTranslated.p[2].z = triRotatedY.p[2].z + offset;

		// discover triangle normal
		vec3d normal, line1, line2;
		line1.x = triTranslated.p[1].x - triTranslated.p[0].x;
		line1.y = triTranslated.p[1].y - triTranslated.p[0].y;
		line1.z = triTranslated.p[1].z - triTranslated.p[0].z;

		line2.x = triTranslated.p[2].x - triTranslated.p[0].x;
		line2.y = triTranslated.p[2].y - triTranslated.p[0].y;
		line2.z = triTranslated.p[2].z - triTranslated.p[0].z;

		normal.x = line1.y * line2.z - line1.z * line2.y;
		normal.y = line1.z * line2.x - line1.x * line2.z;
		normal.z = line1.x * line2.y - line1.y * line2.x;

		float len = sqrtf(normal.x*normal.x + normal.y*normal.y + normal.z*normal.z);
		normal.x /= len; normal.y /= len; normal.z /= len;

		//Only draw triangles that are visible to the 'camera'
		if (normal.x * (triTranslated.p[0].x - vCamera.x) +
			normal.y * (triTranslated.p[0].y - vCamera.y) +
			normal.z * (triTranslated.p[0].z - vCamera.z) < 0.0f) {

			//Create light vector and normalize it
			vec3d light = {0.0f, 0.0f, -1.0f};
			float len = sqrtf(light.x*light.x + light.y*light.y + light.z*light.z);
			light.x /= len; light.y /= len; light.z /= len;

			float dp = normal.x * light.x + normal.y * light.y + normal.z * light.z;
			triTranslated.color = dp;

			//Apply the transformation matrix to it
			multiplyMatrix(triTranslated.p[0], triProjected.p[0], matProj);
			multiplyMatrix(triTranslated.p[1], triProjected.p[1], matProj);
			multiplyMatrix(triTranslated.p[2], triProjected.p[2], matProj);
			triProjected.color = triTranslated.color;

			// Scale the triangle
			triProjected.p[0].x += 1.0f; triProjected.p[0].y += 1.0f;
			triProjected.p[1].x += 1.0f; triProjected.p[1].y += 1.0f;
			triProjected.p[2].x += 1.0f; triProjected.p[2].y += 1.0f;
			
			triProjected.p[0].x *= 0.5f * (float)screenwidth;
			triProjected.p[0].y *= 0.5f * (float)screenheight;
			triProjected.p[1].x *= 0.5f * (float)screenwidth;
			triProjected.p[1].y *= 0.5f * (float)screenheight;
			triProjected.p[2].x *= 0.5f * (float)screenwidth;
			triProjected.p[2].y *= 0.5f * (float)screenheight;

			drawTriangle(triProjected.p[0].x, triProjected.p[0].y,
					triProjected.p[1].x, triProjected.p[1].y,
					triProjected.p[2].x, triProjected.p[2].y,
					0, triProjected.color);
		}
	}

	return 0;
}

