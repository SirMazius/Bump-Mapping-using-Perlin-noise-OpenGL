#version 150 core

struct LightInfo {
		vec4 lightPos; // S.R. del mundo
		vec3 intensity;
	};

struct MaterialInfo {
		vec3 ambient;
		vec3 diffuse;
		vec3 specular;
		float shininess;
	};

uniform LightInfo uLight;
uniform MaterialInfo uMaterial;

uniform sampler1D uPerlinColor;

uniform bool use_normal;
uniform bool use_color;
uniform bool teapot;
uniform bool torus;
uniform bool buddha;
uniform bool show_normal_noise;

uniform int p[512];
uniform int epsilon;


in vec3 vEcPos;
in vec3 vEcNorm;
in vec3 vPosition;
in vec3 vLightDir;
in vec3 vViewDir;
in vec3 vNormal;
in vec2 vST;


out vec4 fFragColor;

const float noise_factor = 1;
const float noise_factor2 = 32;

float f0, fx, fy, fz; //Variables de los factores de ruido

float fade(float t) {
	return t * t * t * (t * (t * 6 - 15) + 10);
}

float lerp(float t, float a, float b) { 
	return a + t * (b - a); 
}

float grad(int hash, float x, float y, float z) {
	int h = hash & 15;
	float u = h < 8 ? x : y;
	float v;

	if (h<4) {
		v = y;
	} else if (h==12||h==14) {
		v = x;
	} else {
		v = z;
	}
		
	return ((h&1) == 0 ? u : -u) + ((h&2) == 0 ? v : -v); 
}

vec3 ads (vec3 _normal, vec3 _vViewDir, vec3 ldir) {
	vec3 r = reflect(-ldir,_normal);
	vec3 specular;
	specular = vec3(0.5);

	if (use_color && !torus)
		return uLight.intensity * (vec3(texture(uPerlinColor, f0)) * 0.2 + vec3(texture(uPerlinColor, f0)) * max(dot(ldir,_normal), 0.0) +  specular * pow(max(dot(r,_vViewDir),0), uMaterial.shininess));

	return uLight.intensity * (uMaterial.ambient + uMaterial.diffuse * max(dot(ldir,_normal), 0.0) +  specular * pow(max(dot(r,_vViewDir),0), uMaterial.shininess));
}

float noise2(float, float, float);//PROTOTIPO



float turb (float x, float y, float z) {
	float value = 0;
	float mag = 1;
	float amplitude = 1;

	for (int i = 0; i < 10; i++) {
		value += amplitude * noise2(x*mag, y*mag, z*mag);
		amplitude *= 0.5;
		mag *= 2;
	}

	return clamp(abs(value),0,1);
}


const float a = 0.8; //Valor para controlar la amplitud del ruido
const float f = 5; // Valor para controlar la frecuencia

float marble(float x, float y, float z) {
	return sin(f*(x+a*turb(x,y,z)));
}

float noise2 (float x, float y, float z) {
	int X = int(int(floor(x)) & 255),
		Y = int(int(floor(y)) & 255),
		Z = int(int(floor(z)) & 255);

		x -= floor(x);
		y -= floor(y);
		z -= floor(z);

		float u = fade(x), 
		v = fade(y),
		w = fade(z);

		int A = p[X]+Y, AA = p[A]+Z, AB = p[A+1]+Z,
			B = p[X+1]+Y, BA = p[B]+Z, BB = p[B+1]+Z;

	return lerp(w, lerp(v, lerp(u, grad(p[AA  ], x  , y  , z   ),  
                                     grad(p[BA  ], x-1, y  , z   )),
                             lerp(u, grad(p[AB  ], x  , y-1, z   ),  
                                     grad(p[BB  ], x-1, y-1, z   ))),
                     lerp(v, lerp(u, grad(p[AA+1], x  , y  , z-1 ),  
                                     grad(p[BA+1], x-1, y  , z-1 )), 
                             lerp(u, grad(p[AB+1], x  , y-1, z-1 ),
                                     grad(p[BB+1], x-1, y-1, z-1 ))));
}


float gestiona_ruido(float x, float y, float z) {
	float value = noise2(x*noise_factor*50, y*noise_factor, z*noise_factor);
	return value;
}

float gestiona_ruido2(float x, float y, float z) {
	float value = noise2(x*noise_factor2, y*noise_factor2, z*noise_factor2);
	return value;
}



void main() {
	vec3 normal;
	float scale = 0.04;
	float bias = -0.02;

	normal = vec3(0,0,1);
	vec3 dF = vec3(0,0,0);

	if (use_normal) {
		if (torus) { //Si usamos el toro
			f0 = gestiona_ruido(vPosition.x, vPosition.y, vPosition.z);
			fx = gestiona_ruido(vPosition.x + epsilon, vPosition.y, vPosition.z);
			fy = gestiona_ruido(vPosition.x, vPosition.y + epsilon, vPosition.z);
			fz = gestiona_ruido(vPosition.x, vPosition.y , vPosition.z + epsilon);
			dF = vec3((fx - f0) / epsilon, (fy - f0) / epsilon, (fz - f0) / epsilon); //desplazamiento para el bump mapping
		} else if (buddha) { //Si usamos el buddha
			f0 = gestiona_ruido2(vPosition.x, vPosition.y, vPosition.z);
			fx = gestiona_ruido2(vPosition.x + epsilon, vPosition.y, vPosition.z);
			fy = gestiona_ruido2(vPosition.x, vPosition.y + epsilon, vPosition.z);
			fz = gestiona_ruido2(vPosition.x, vPosition.y , vPosition.z + epsilon);
			dF = vec3((fx - f0) / epsilon, (fy - f0) / epsilon, (fz - f0) / epsilon); //desplazamiento para el bump mapping
		}
	}
		
	f0 = marble(vPosition.x, vPosition.y, vPosition.z);

	if (!show_normal_noise || teapot) 
		fFragColor = vec4(ads(normalize(normal - dF) , normalize(vViewDir), normalize(vLightDir)), 1.);
	else {
			if (torus)
				f0 = gestiona_ruido(vPosition.x, vPosition.y, vPosition.z);//veteado
			else
				f0 = gestiona_ruido2(vPosition.x, vPosition.y, vPosition.z);//alta frecuencia

			fFragColor = vec4(f0, f0, f0, 1);
		}	
}