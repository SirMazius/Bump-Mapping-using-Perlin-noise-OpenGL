// Ej3parte3.cpp: define el punto de entrada de la aplicación de consola.
//

#include "stdafx.h"

using namespace std;

int initPlane();
int initTorus(float outerRadius, float innerRadius, int nsides, int nrings);
void drawPlane();
void drawTorus();

void loadSource(GLuint &shaderID, std::string name);
void printCompileInfoLog(GLuint shadID);
void printLinkInfoLog(GLuint programID);
void validateProgram(GLuint programID);

void load_obj(const char*, std::vector<glm::vec4> &, std::vector<glm::vec3> &, std::vector<GLushort> &);

bool init();
bool wireframe = false;
void display();
void resize(int, int);
void idle();
void keyboard(unsigned char, int, int);
void specialKeyboard(int, int, int);
void mouse(int, int, int, int);
void mouseMotion(int, int);


bool fullscreen = false;
bool mouseDown = false;
bool animation = true;
bool use_normal_map = false;
bool use_color = false;
bool use_gloss = false;
bool parallex_simple = false;
bool parallex_iterative = false;
bool teapot = true;
bool torus = false;
bool buddha = false;
bool show_noise = false;

float xrot = 0.0f;
float yrot = 0.0f;
float xdiff = 0.0f;
float ydiff = 0.0f;

const int g_Width = 1920;                          // Ancho inicial de la ventana
const int g_Height = 1080;                         // Altura incial de la ventana

const int permutation[]{ 151,160,137,91,90,15,
131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180 };

int p[512];
int epsilon = 2;

GLubyte MatrixImage[256][3];

GLuint cubeVAOHandle, sphereVAOHandle, teapotVAOHandle, planeVAOHandle, torusVAOHandle;
GLuint programID;
GLuint locUniformMVPM, locUniformMVM, locUniformNM;
GLuint locUniformLightPos, locUniformLightIntensity;
GLuint locUniformMaterialAmbient, locUniformMaterialDiffuse, locUniformMaterialSpecular, locUniformMaterialShininess;
GLuint locUniformNormalMap, locUniformColorMap, locUniformGlossMap, locUniformHeightMap, locUniformUseNormal, locUniformUseColor,
locUniformUseGloss, locUniformParallexSimple, locUniformParallexIterative, 
locUniformPerlinColor, locUniformPermutation, locUniformP, locUniformTeapot, locUniformTorus, locUniformBuddha, locUniformEpsilon, locUniformshowNormal, objVAOHandle,locUniformMM;

GLuint FrameBuffer;
GLuint TexturePerlin;

int numVertTeapot, numVertSphere, numVertPlane, numVertTorus, numVertObj;

GLuint textIds[4];

std::vector<GLfloat> obj_vertices;
std::vector<GLfloat> obj_normals;
std::vector<GLushort> obj_elements;

void load_obj(const char* filename, std::vector<GLfloat> &vertices, std::vector<GLfloat> &normals, std::vector<GLushort> &elements) {
	std::ifstream in(filename, std::ios::in);
	if (!in) { std::cerr << "Cannot open " << filename << std::endl; std::system("pause"); exit(1); }

	std::string line;
	while (getline(in, line)) {
		if (line.substr(0, 2) == "v ") {
			std::istringstream s(line.substr(2));
			GLfloat x, y, z; s >> x; s >> y; s >> z;
			vertices.push_back(x);
			vertices.push_back(y);
			vertices.push_back(z);
		}
		else if (line.substr(0, 2) == "f ") {
			std::istringstream s(line.substr(2));
			GLushort a, b, c;
			s >> a; s >> b; s >> c;
			a--; b--; c--;
			elements.push_back(a); elements.push_back(b); elements.push_back(c);
		}
		else if (line[0] == '#') { /* ignoring this line */ }
		else { /* ignoring this line */ }
	}

	normals.resize(vertices.size(), 0.0f);
	for (int i = 0; i < elements.size(); i += 3) {
		GLushort ia = elements[i];
		GLushort ib = elements[i + 1];
		GLushort ic = elements[i + 2];
		glm::vec3 normal = glm::normalize(glm::cross(
			glm::vec3(vertices[ib * 3 + 0], vertices[ib * 3 + 1], vertices[ib * 3 + 2]) - glm::vec3(vertices[ia * 3 + 0], vertices[ia * 3 + 1], vertices[ia * 3 + 2]),
			glm::vec3(vertices[ic * 3 + 0], vertices[ic * 3 + 1], vertices[ic * 3 + 2]) - glm::vec3(vertices[ia * 3 + 0], vertices[ia * 3 + 1], vertices[ia * 3 + 2])));
		normals[ia * 3 + 0] = normals[ib * 3 + 0] = normals[ic * 3 + 0] = normal.x;
		normals[ia * 3 + 1] = normals[ib * 3 + 1] = normals[ic * 3 + 1] = normal.y;
		normals[ia * 3 + 2] = normals[ib * 3 + 2] = normals[ic * 3 + 2] = normal.z;
	}
}

int initObj(std::string file)
{
	load_obj(file.c_str(), obj_vertices, obj_normals, obj_elements);

	glGenVertexArrays(1, &objVAOHandle);
	glBindVertexArray(objVAOHandle);

	unsigned int handle[3];
	glGenBuffers(3, handle);

	glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
	glBufferData(GL_ARRAY_BUFFER, obj_vertices.size() * sizeof(GLfloat), obj_vertices.data(), GL_STATIC_DRAW); // Datos de la posición de los vértices
	GLuint loc1 = glGetAttribLocation(programID, "aPosition");
	glEnableVertexAttribArray(loc1); // Vertex position
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, (GLubyte *)NULL + 0);

	glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
	glBufferData(GL_ARRAY_BUFFER, obj_normals.size() * sizeof(GLfloat), obj_normals.data(), GL_STATIC_DRAW); // Datos de las normales de los vértices
	GLuint loc2 = glGetAttribLocation(programID, "aNormal");
	glEnableVertexAttribArray(loc2); // Vertex normal
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, (GLubyte *)NULL + 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, obj_elements.size() * sizeof(GLushort), obj_elements.data(), GL_STATIC_DRAW); // Array de índices

	glBindVertexArray(0);

	return obj_elements.size();
}

void drawObj() {
	glBindVertexArray(objVAOHandle);
	glDrawElements(GL_TRIANGLES, numVertObj, GL_UNSIGNED_SHORT, ((GLubyte *)NULL + (0)));
	glBindVertexArray(0);
}


// BEGIN: Carga shaders ////////////////////////////////////////////////////////////////////////////////////////////

void loadSource(GLuint &shaderID, std::string name)
{
	std::ifstream f(name.c_str());
	if (!f.is_open())
	{
		std::cerr << "File not found " << name.c_str() << std::endl;
		system("pause");
		exit(EXIT_FAILURE);
	}

	// now read in the data
	std::string *source;
	source = new std::string(std::istreambuf_iterator<char>(f),
		std::istreambuf_iterator<char>());
	f.close();

	// add a null to the string
	*source += "\0";
	const GLchar * data = source->c_str();
	glShaderSource(shaderID, 1, &data, NULL);
	delete source;
}

void printCompileInfoLog(GLuint shadID)
{
	GLint compiled;
	glGetShaderiv(shadID, GL_COMPILE_STATUS, &compiled);
	if (compiled == GL_FALSE)
	{
		GLint infoLength = 0;
		glGetShaderiv(shadID, GL_INFO_LOG_LENGTH, &infoLength);

		GLchar *infoLog = new GLchar[infoLength];
		GLint chsWritten = 0;
		glGetShaderInfoLog(shadID, infoLength, &chsWritten, infoLog);

		std::cerr << "Shader compiling failed:" << infoLog << std::endl;
		system("pause");
		delete[] infoLog;

		exit(EXIT_FAILURE);
	}
}

void printLinkInfoLog(GLuint programID)
{
	GLint linked;
	glGetProgramiv(programID, GL_LINK_STATUS, &linked);
	if (!linked)
	{
		GLint infoLength = 0;
		glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLength);

		GLchar *infoLog = new GLchar[infoLength];
		GLint chsWritten = 0;
		glGetProgramInfoLog(programID, infoLength, &chsWritten, infoLog);

		std::cerr << "Shader linking failed:" << infoLog << std::endl;
		system("pause");
		delete[] infoLog;

		exit(EXIT_FAILURE);
	}
}

void validateProgram(GLuint programID)
{
	GLint status;
	glValidateProgram(programID);
	glGetProgramiv(programID, GL_VALIDATE_STATUS, &status);

	if (status == GL_FALSE)
	{
		GLint infoLength = 0;
		glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLength);

		if (infoLength > 0)
		{
			GLchar *infoLog = new GLchar[infoLength];
			GLint chsWritten = 0;
			glGetProgramInfoLog(programID, infoLength, &chsWritten, infoLog);
			std::cerr << "Program validating failed:" << infoLog << std::endl;
			system("pause");
			delete[] infoLog;

			exit(EXIT_FAILURE);
		}
	}
}

// END:   Carga shaders ////////////////////////////////////////////////////////////////////////////////////////////

// BEGIN: Inicializa primitivas ////////////////////////////////////////////////////////////////////////////////////

int initPlane(float xsize, float zsize, int xdivs, int zdivs)
{
	const float TexScale = 2.0f;
	float * v = new float[3 * (xdivs + 1) * (zdivs + 1)];
	float * n = new float[3 * (xdivs + 1) * (zdivs + 1)];
	float * t = new float[3 * (xdivs + 1) * (zdivs + 1)];
	float * tex = new float[2 * (xdivs + 1) * (zdivs + 1)];
	unsigned int * el = new unsigned int[6 * xdivs * zdivs];

	float x2 = xsize / 2.0f;
	float z2 = zsize / 2.0f;
	float iFactor = (float)zsize / zdivs;
	float jFactor = (float)xsize / xdivs;
	float texi = 1.0f / zdivs;
	float texj = 1.0f / xdivs;
	float x, z;
	int vidx = 0, tidx = 0;
	for (int i = 0; i <= zdivs; i++) {
		z = iFactor * i - z2;
		for (int j = 0; j <= xdivs; j++) {
			x = jFactor * j - x2;
			v[vidx] = x;
			v[vidx + 1] = 0.0f;
			v[vidx + 2] = z;
			n[vidx] = 0.0f;
			n[vidx + 1] = 1.0f;
			n[vidx + 2] = 0.0f;
			t[vidx] = -1.0f;
			t[vidx + 1] = 0.0f;
			t[vidx + 2] = 0.0f;
			vidx += 3;
			tex[tidx] = -j * texi * TexScale;
			tex[tidx + 1] = i * texj * TexScale;
			tidx += 2;
		}
	}

	unsigned int rowStart, nextRowStart;
	int idx = 0;
	for (int i = 0; i < zdivs; i++) {
		rowStart = i * (xdivs + 1);
		nextRowStart = (i + 1) * (xdivs + 1);
		for (int j = 0; j < xdivs; j++) {
			el[idx] = rowStart + j;
			el[idx + 1] = nextRowStart + j;
			el[idx + 2] = nextRowStart + j + 1;
			el[idx + 3] = rowStart + j;
			el[idx + 4] = nextRowStart + j + 1;
			el[idx + 5] = rowStart + j + 1;
			idx += 6;
		}
	}

	unsigned int handle[5];
	glGenBuffers(5, handle);

	glGenVertexArrays(1, &planeVAOHandle);
	glBindVertexArray(planeVAOHandle);

	glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
	glBufferData(GL_ARRAY_BUFFER, 3 * (xdivs + 1) * (zdivs + 1) * sizeof(float), v, GL_STATIC_DRAW);
	GLuint loc1 = glGetAttribLocation(programID, "aPosition");
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, ((GLubyte *)NULL + (0)));
	glEnableVertexAttribArray(loc1);  // Vertex position

	glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
	glBufferData(GL_ARRAY_BUFFER, 3 * (xdivs + 1) * (zdivs + 1) * sizeof(float), n, GL_STATIC_DRAW);
	GLuint loc2 = glGetAttribLocation(programID, "aNormal");
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, ((GLubyte *)NULL + (0)));
	glEnableVertexAttribArray(loc2);  // Vertex normal

	glBindBuffer(GL_ARRAY_BUFFER, handle[2]);
	glBufferData(GL_ARRAY_BUFFER, 3 * (xdivs + 1) * (zdivs + 1) * sizeof(float), t, GL_STATIC_DRAW);
	GLuint loc3 = glGetAttribLocation(programID, "aTangent");
	glVertexAttribPointer(loc3, 3, GL_FLOAT, GL_FALSE, 0, ((GLubyte *)NULL + (0)));
	glEnableVertexAttribArray(loc3);  // Vertex tangent

	glBindBuffer(GL_ARRAY_BUFFER, handle[3]);
	glBufferData(GL_ARRAY_BUFFER, 2 * (xdivs + 1) * (zdivs + 1) * sizeof(float), tex, GL_STATIC_DRAW);
	GLuint loc4 = glGetAttribLocation(programID, "aTexCoord");
	glVertexAttribPointer(loc4, 2, GL_FLOAT, GL_FALSE, 0, ((GLubyte *)NULL + (0)));
	glEnableVertexAttribArray(loc4);  // texture coords

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle[4]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * xdivs * zdivs * sizeof(unsigned int), el, GL_STATIC_DRAW);

	glBindVertexArray(0);

	delete[] v;
	delete[] n;
	delete[] t;
	delete[] tex;
	delete[] el;

	return 6 * xdivs * zdivs;
}

int initTorus(float outerRadius, float innerRadius, int nsides, int nrings)
{
	int faces = nsides * nrings;
	int nVerts = nsides * (nrings + 1);   // One extra ring to duplicate first ring

										  // Verts
	float * v = new float[3 * nVerts];
	// Normals
	float * n = new float[3 * nVerts];
	// Tangent
	float * t = new float[3 * nVerts];
	// Tex coords
	float * tex = new float[2 * nVerts];
	// Elements
	unsigned int * el = new unsigned int[6 * faces];

	// Generate the vertex data
	generateVerts(v, n, t, tex, el, outerRadius, innerRadius, nrings, nsides);

	// Create and populate the buffer objects
	unsigned int handle[5];
	glGenBuffers(5, handle);

	// Create the VAO
	glGenVertexArrays(1, &torusVAOHandle);
	glBindVertexArray(torusVAOHandle);

	glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
	glBufferData(GL_ARRAY_BUFFER, (3 * nVerts) * sizeof(float), v, GL_STATIC_DRAW);
	GLuint loc1 = glGetAttribLocation(programID, "aPosition");
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, ((GLubyte *)NULL + (0)));
	glEnableVertexAttribArray(loc1);  // Vertex position

	glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
	glBufferData(GL_ARRAY_BUFFER, (3 * nVerts) * sizeof(float), n, GL_STATIC_DRAW);
	GLuint loc2 = glGetAttribLocation(programID, "aNormal");
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, ((GLubyte *)NULL + (0)));
	glEnableVertexAttribArray(loc2);  // Vertex normal

	glBindBuffer(GL_ARRAY_BUFFER, handle[2]);
	glBufferData(GL_ARRAY_BUFFER, (3 * nVerts) * sizeof(float), t, GL_STATIC_DRAW);
	GLuint loc3 = glGetAttribLocation(programID, "aTangent");
	glVertexAttribPointer(loc3, 3, GL_FLOAT, GL_FALSE, 0, ((GLubyte *)NULL + (0)));
	glEnableVertexAttribArray(loc3);  // Vertex tangent

	glBindBuffer(GL_ARRAY_BUFFER, handle[3]);
	glBufferData(GL_ARRAY_BUFFER, (2 * nVerts) * sizeof(float), tex, GL_STATIC_DRAW);
	GLuint loc4 = glGetAttribLocation(programID, "aTexCoord");
	glVertexAttribPointer(loc4, 2, GL_FLOAT, GL_FALSE, 0, ((GLubyte *)NULL + (0)));
	glEnableVertexAttribArray(loc4);  // texture coords

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle[4]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * faces * sizeof(unsigned int), el, GL_STATIC_DRAW);

	glBindVertexArray(0);

	delete[] v;
	delete[] n;
	delete[] t;
	delete[] el;
	delete[] tex;

	return 6 * faces;
}

int initTeapot(int grid, glm::mat4 transform)
{
	int verts = 32 * (grid + 1) * (grid + 1);
	int faces = grid * grid * 32;
	float * v = new float[verts * 3];
	float * n = new float[verts * 3];
	float * tc = new float[verts * 2];
	unsigned int * el = new unsigned int[faces * 6];

	generatePatches(v, n, tc, el, grid);
	moveLid(grid, v, transform);

	glGenVertexArrays(1, &teapotVAOHandle);
	glBindVertexArray(teapotVAOHandle);

	unsigned int handle[4];
	glGenBuffers(4, handle);

	glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
	glBufferData(GL_ARRAY_BUFFER, (3 * verts) * sizeof(float), v, GL_STATIC_DRAW); // Datos de la posición de los vértices
	GLuint loc1 = glGetAttribLocation(programID, "aPosition");
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, ((GLubyte *)NULL + (0)));
	glEnableVertexAttribArray(loc1);  // Vertex position

	glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
	glBufferData(GL_ARRAY_BUFFER, (3 * verts) * sizeof(float), n, GL_STATIC_DRAW); // Datos de las normales de los vértices
	GLuint loc2 = glGetAttribLocation(programID, "aNormal");
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, ((GLubyte *)NULL + (0)));
	glEnableVertexAttribArray(loc2);  // Vertex normal

	glBindBuffer(GL_ARRAY_BUFFER, handle[2]);
	glBufferData(GL_ARRAY_BUFFER, (2 * verts) * sizeof(float), tc, GL_STATIC_DRAW); // Datos de las coordenadas de textura
	GLuint loc3 = glGetAttribLocation(programID, "aTexCoord");
	glVertexAttribPointer(loc3, 2, GL_FLOAT, GL_FALSE, 0, ((GLubyte *)NULL + (0)));
	glEnableVertexAttribArray(loc3);  // texture coords

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle[3]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * faces * sizeof(unsigned int), el, GL_STATIC_DRAW); // Array de índices

	delete[] v;
	delete[] n;
	delete[] el;
	delete[] tc;

	glBindVertexArray(0);

	return 6 * faces;
}

// END: Inicializa primitivas ////////////////////////////////////////////////////////////////////////////////////

// BEGIN: Funciones de dibujo ////////////////////////////////////////////////////////////////////////////////////

void drawPlane() {
	glBindVertexArray(planeVAOHandle);
	glDrawElements(GL_TRIANGLES, numVertPlane, GL_UNSIGNED_INT, ((GLubyte *)NULL + (0)));
	glBindVertexArray(0);
}

void drawTorus() {
	glBindVertexArray(torusVAOHandle);
	glDrawElements(GL_TRIANGLES, numVertTorus, GL_UNSIGNED_INT, ((GLubyte *)NULL + (0)));
	glBindVertexArray(0);
}

void drawTeapot() {
	glBindVertexArray(teapotVAOHandle);
	glDrawElements(GL_TRIANGLES, numVertTeapot, GL_UNSIGNED_INT, ((GLubyte *)NULL + (0)));
	glBindVertexArray(0);
}
// END: Funciones de dibujo ////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitWindowPosition(50, 50);
	glutInitWindowSize(g_Width, g_Height);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow("Programa Ejemplo");
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
		system("pause");
		exit(-1);
	}
	init();

	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(specialKeyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(mouseMotion);
	glutReshapeFunc(resize);
	glutIdleFunc(idle);

	glutMainLoop();

	return EXIT_SUCCESS;
}

bool init()
{
	glClearColor(0.93f, 0.93f, 0.93f, 0.0f);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glClearDepth(1.0f);

	glShadeModel(GL_SMOOTH);

	programID = glCreateProgram();

	

	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	loadSource(vertexShaderID, "tema3_parte3.vert");
	glCompileShader(vertexShaderID);
	printCompileInfoLog(vertexShaderID);
	glAttachShader(programID, vertexShaderID);

	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	loadSource(fragmentShaderID, "tema3_parte3.frag");
	glCompileShader(fragmentShaderID);
	printCompileInfoLog(fragmentShaderID);
	glAttachShader(programID, fragmentShaderID);

	glLinkProgram(programID);
	printLinkInfoLog(programID);
	validateProgram(programID);

	numVertPlane = initPlane(10.0f, 10.0f, 4, 4);
	numVertTorus = initTorus(0.5f, 0.25f, 40, 40);
	numVertTeapot = initTeapot(5, glm::mat4(1.0f));
	numVertObj = initObj("buddha.obj");
	locUniformMVPM = glGetUniformLocation(programID, "uModelViewProjMatrix");
	locUniformMVM = glGetUniformLocation(programID, "uModelViewMatrix");
	locUniformNM = glGetUniformLocation(programID, "uNormalMatrix");

	locUniformLightPos = glGetUniformLocation(programID, "uLight.lightPos");
	locUniformLightIntensity = glGetUniformLocation(programID, "uLight.intensity");
	locUniformMaterialAmbient = glGetUniformLocation(programID, "uMaterial.ambient");
	locUniformMaterialDiffuse = glGetUniformLocation(programID, "uMaterial.diffuse");
	locUniformMaterialSpecular = glGetUniformLocation(programID, "uMaterial.specular");
	locUniformMaterialShininess = glGetUniformLocation(programID, "uMaterial.shininess");

	locUniformNormalMap = glGetUniformLocation(programID, "uNormalMap");
	locUniformColorMap = glGetUniformLocation(programID, "uColorMap");
	locUniformPerlinColor = glGetUniformLocation(programID, "uPerlinColor");

	locUniformUseNormal = glGetUniformLocation(programID, "use_normal");
	locUniformUseColor = glGetUniformLocation(programID, "use_color");

	locUniformPermutation = glGetUniformLocation(programID, "permutation");
	locUniformP = glGetUniformLocation(programID, "p");
	locUniformEpsilon = glGetUniformLocation(programID, "epsilon");
	locUniformTeapot = glGetUniformLocation(programID, "teapot");
	locUniformTorus = glGetUniformLocation(programID, "torus");
	locUniformBuddha = glGetUniformLocation(programID, "buddha");
	locUniformMM = glGetUniformLocation(programID, "uModelMatrix");
	locUniformshowNormal = glGetUniformLocation(programID, "show_normal_noise");


	// Inicializa la textura de color
	TGAFILE tgaImage;
	glGenTextures(5, textIds);

	/*
		Usamos dos bucles para cubrir la parte de las frecuencias bajas con valores mas altos 
		para enmascarar el ruido
	*/
	const int increase = 135;
	for (int i = 0; i < 75; i++) {
		MatrixImage[i][0] = i + increase;
		MatrixImage[i][1] = i * 0.85 + increase;
		MatrixImage[i][2] = i * 0.5 + increase;
	}

	for (int i = 75; i < 256; i++) {
		MatrixImage[i][0] = i;
		MatrixImage[i][1] = i*0.85;
		MatrixImage[i][2] = i*0.75;
	}


	//Textura de color 1D 
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_1D, textIds[4]);
	gluBuild1DMipmaps(GL_TEXTURE_1D,			// texture to specify
		GL_RGB,				// internal texture storage format
		256,   // texture width
		GL_RGB,				// pixel format
		GL_UNSIGNED_BYTE,		// color component format
		MatrixImage);	// pointer to texture image
	glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT);


	/*
		Rellenamos p
	*/

	for (int i = 0; i < 256; i++)
		p[256 + i] = p[i] = permutation[i];

	return true;
}

void display()
{
	struct LightInfo {
		glm::vec4 lightPos; // Pos. S.R. del mundo
		glm::vec3 intensity;
	};
	LightInfo light = { glm::vec4(4.0f, 4.0f, 4.0f, 1.0f), // Pos. S.R. del mundo
		glm::vec3(1.0f, 1.0f, 1.0f)
	};

	struct MaterialInfo {
		glm::vec3 ambient;
		glm::vec3 diffuse;
		glm::vec3 specular;
		GLfloat shininess;
	};
	MaterialInfo gold = { glm::vec3(0.24725f, 0.1995f, 0.0745f), glm::vec3(0.75164f, 0.60648f, 0.22648f), glm::vec3(0.628281f, 0.555802f, 0.366065f), 52.0f };
	MaterialInfo perl = { glm::vec3(0.25f, 0.20725f, 0.20725f), glm::vec3(1.0f, 0.829f, 0.829f), glm::vec3(0.296648f, 0.296648f, 0.296648f), 12.0f };
	MaterialInfo bronze = { glm::vec3(0.2125f, 0.1275f, 0.054f), glm::vec3(0.714f, 0.4284f, 0.18144f), glm::vec3(0.393548f, 0.271906f, 0.166721f), 25.0f };
	MaterialInfo brass = { glm::vec3(0.329412f, 0.223529f, 0.027451f), glm::vec3(0.780392f, 0.568627f, 0.113725f), glm::vec3(0.992157f, 0.941176f, 0.807843f), 28.0f };
	MaterialInfo emerald = { glm::vec3(0.0215f, 0.1745f, 0.0215f), glm::vec3(0.07568f, 0.61424f, 0.07568f), glm::vec3(0.633f, 0.727811f, 0.633f), 28.0f };

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 Projection = glm::perspective(45.0f, 1.0f * g_Width / g_Height, 1.0f, 100.0f);

	glm::vec3 cameraPos = glm::vec3(5.0f * sin(xrot / 200) * cos(yrot / 100) + 1.8f, 2.2f * cos(xrot / 200) + 2.5f, 5.0f * sin(xrot / 200) * sin(yrot / 100) + 1.8f);
	glm::mat4 View = glm::lookAt(cameraPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	glm::mat4 ModelPlane = glm::translate(glm::scale(glm::mat4(1.0), glm::vec3(1.0f, 1.0f, 1.0f)), glm::vec3(0.0f, 0.0f, 0.0f));
	glm::mat4 ModelTorus = glm::translate(glm::scale(glm::rotate(glm::mat4(1.0f), -90.0f, glm::vec3(1.0, 0.0, 0.0)), glm::vec3(2.0, 2.0, 2.0)), glm::vec3(0.0f, 0.0f, 0.25f));
	glm::mat4 ModelTeapot = glm::translate(glm::rotate(glm::scale(glm::mat4(1.0f), vec3(0.55, 0.55, 0.55)), -90.0f, vec3(1.0, 0.0, 0.0)), vec3(0.0f, 0.0f, 0.0f));
	glm::mat4 ModelObj = glm::translate(glm::rotate(glm::mat4(1.0f), -90.0f, vec3(1.0, 0.0, 0.0)), vec3(-1.0f, 0.0f, 0.0f));

	glm::mat4 mvp; // Model-view-projection matrix
	glm::mat4 mv;  // Model-view matrix
	glm::mat3 nm;  // Normal matrix

	glUseProgram(programID);

	glUniform1i(locUniformNormalMap, 0);	// GL_TEXURE0
	glUniform1i(locUniformColorMap, 1);	// GL_TEXURE1
	glUniform1i(locUniformGlossMap, 2);	// GL_TEXURE2
	glUniform1i(locUniformHeightMap, 3);	// GL_TEXURE3
	glUniform1i(locUniformPerlinColor, 4);
	glUniform1i(locUniformEpsilon, epsilon);

	glUniform1i(locUniformUseNormal, use_normal_map);
	glUniform1i(locUniformUseColor, use_color);
	glUniform1i(locUniformParallexSimple, parallex_simple);
	glUniform1i(locUniformParallexIterative, parallex_iterative);
	glUniform1iv(locUniformP, 512, p);
	glUniform1i(locUniformTeapot, teapot);
	glUniform1i(locUniformTorus, torus);
	glUniform1i(locUniformBuddha, buddha);
	glUniform1i(locUniformshowNormal, show_noise);
											// Paso al shader de las propiedades de la luz
	glm::vec4 lpos = View * light.lightPos;	// Posición del foco en el S.R. vista
	glUniform4fv(locUniformLightPos, 1, &(lpos.x));
	glUniform3fv(locUniformLightIntensity, 1, &(light.intensity.r));

	// Dibuja Donut
	mvp = Projection * View * ModelTorus;
	mv = View * ModelTorus;
	nm = glm::mat3(glm::transpose(glm::inverse(mv)));
	glUniformMatrix4fv(locUniformMVPM, 1, GL_FALSE, &mvp[0][0]);
	glUniformMatrix4fv(locUniformMVM, 1, GL_FALSE, &mv[0][0]);
	glUniformMatrix3fv(locUniformNM, 1, GL_FALSE, &nm[0][0]);
	glUniform3fv(locUniformMaterialAmbient, 1, &(emerald.ambient.r));
	glUniform3fv(locUniformMaterialDiffuse, 1, &(emerald.diffuse.r));
	glUniform3fv(locUniformMaterialSpecular, 1, &(emerald.specular.r));
	glUniform1f(locUniformMaterialShininess, emerald.shininess);
	if (torus)
		drawTorus();

	//Dibuja teapot
	mvp = Projection * View * ModelTeapot;
	mv = View * ModelTeapot;
	nm = glm::mat3(glm::transpose(glm::inverse(mv)));

	glUniformMatrix4fv(locUniformMVPM, 1, GL_FALSE, &mvp[0][0]);
	glUniformMatrix4fv(locUniformMVM, 1, GL_FALSE, &mv[0][0]);
	glUniformMatrix3fv(locUniformNM, 1, GL_FALSE, &nm[0][0]);

	glUniform3fv(locUniformMaterialAmbient, 1, &(emerald.ambient.r));
	glUniform3fv(locUniformMaterialDiffuse, 1, &(emerald.diffuse.r));
	glUniform3fv(locUniformMaterialSpecular, 1, &(emerald.specular.r));
	glUniform1f(locUniformMaterialShininess, brass.shininess);
	// Tarea por hacer: paso al shader de las matrices y las propiedades del material (brass)
	// Dibuja Tetera
	if (teapot)
		drawTeapot();

	 //Dibuja Plano
	mvp = Projection * View * ModelPlane;
	mv = View * ModelPlane;
	nm = glm::mat3(glm::transpose(glm::inverse(mv)));
	glUniformMatrix4fv(locUniformMVPM, 1, GL_FALSE, &mvp[0][0]);
	glUniformMatrix4fv(locUniformMVM, 1, GL_FALSE, &mv[0][0]);
	glUniformMatrix3fv(locUniformNM, 1, GL_FALSE, &nm[0][0]);
	glUniform3fv(locUniformMaterialAmbient, 1, &(brass.ambient.r));
	glUniform3fv(locUniformMaterialDiffuse, 1, &(brass.diffuse.r));
	glUniform3fv(locUniformMaterialSpecular, 1, &(brass.specular.r));
	glUniform1f(locUniformMaterialShininess, brass.shininess);
	//drawPlane();

	//DIBUJA OBJETO
	mvp = Projection * View * ModelObj;
	mv = View * ModelObj;
	nm = glm::mat3(glm::transpose(glm::inverse(mv)));
	glUniformMatrix4fv(locUniformMVPM, 1, GL_FALSE, &mvp[0][0]);
	glUniformMatrix4fv(locUniformMVM, 1, GL_FALSE, &mv[0][0]);
	glUniformMatrix3fv(locUniformNM, 1, GL_FALSE, &nm[0][0]);
	//glUniformMatrix4fv(locUniformMM, 1, GL_FALSE, &ModelObj[0][0]);
	//glUniform1i(locUniformDrawEnvironment, 0);
	glUniform3fv(locUniformMaterialAmbient, 1, &(bronze.ambient.r));
	glUniform3fv(locUniformMaterialDiffuse, 1, &(bronze.diffuse.r));
	glUniform3fv(locUniformMaterialSpecular, 1, &(bronze.specular.r));
	glUniform1f(locUniformMaterialShininess, bronze.shininess);
	if (buddha)
		drawObj();

	glUseProgram(0);

	//glDrawPixels(g_Width, g_Height, GL_RGB, GL_UNSIGNED_BYTE, MatrixImage);

	glutSwapBuffers();
}

void resize(int w, int h)
{
	/*g_Width = w;
	g_Height = h;*/
	glViewport(0, 0, g_Width, g_Height);
}

void idle()
{
	if (!mouseDown && animation)
	{
		xrot += 0.3f;
		yrot += 0.4f;
	}
	glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27: case 'q': case 'Q':
		exit(1);
		break;
	case 'a': case 'A':
		animation = !animation;
		break;
	case 'n': case 'N':
		if (use_normal_map)
			use_normal_map = false;
		else
			use_normal_map = true;
		break;
	case 'c': case 'C':
		if (use_color)
			use_color = false;
		else
			use_color = true;
		break;
	

	case 'w': case 'W':
		wireframe = !wireframe;
		if (wireframe)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		break;

	case 's': case 'S':
		if (show_noise)
			show_noise = false;
		else
			show_noise = true;
		break;
	case '6':
		epsilon += 1;
		break;
	case '4':
		epsilon -= 1;
		break;
	case '1':
		teapot = true;
		torus = false;
		buddha = false;	
		break;
	case '2':
		buddha = true;
		torus = false;
		teapot = false;
		break;
	case '3':
		torus = true;
		buddha = false;
		teapot = false;
	}

	
}

void specialKeyboard(int key, int x, int y)
{
	if (key == GLUT_KEY_F1)
	{
		fullscreen = !fullscreen;

		if (fullscreen)
			glutFullScreen();
		else
		{
			glutReshapeWindow(g_Width, g_Height);
			glutPositionWindow(50, 50);
		}
	}
}

void mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		mouseDown = true;

		xdiff = x - yrot;
		ydiff = -y + xrot;
	}
	else
		mouseDown = false;
}

void mouseMotion(int x, int y)
{
	if (mouseDown)
	{
		yrot = x - xdiff;
		xrot = y + ydiff;

		glutPostRedisplay();
	}
}




