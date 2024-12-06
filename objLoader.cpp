#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <GL/glut.h>

// Global variables for storing the loaded model data
std::vector<GLfloat> vertices;
std::vector<GLfloat> uvs;
std::vector<GLfloat> normals;
GLuint textureID;

// Function to load an OBJ file
bool LoadOBJ(const char* path) {
    std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
    std::vector<GLfloat> temp_vertices;
    std::vector<GLfloat> temp_uvs;
    std::vector<GLfloat> temp_normals;

    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to open OBJ file: " << path << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "v") {
            GLfloat x, y, z;
            iss >> x >> y >> z;
            temp_vertices.push_back(x);
            temp_vertices.push_back(y);
            temp_vertices.push_back(z);
        } else if (prefix == "vt") {
            GLfloat u, v;
            iss >> u >> v;
            temp_uvs.push_back(u);
            temp_uvs.push_back(v);
        } else if (prefix == "vn") {
            GLfloat nx, ny, nz;
            iss >> nx >> ny >> nz;
            temp_normals.push_back(nx);
            temp_normals.push_back(ny);
            temp_normals.push_back(nz);
        } else if (prefix == "f") {
            unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
            char slash; // For parsing the slashes in the face definitions
            for (int i = 0; i < 3; i++) {
                iss >> vertexIndex[i] >> slash >> uvIndex[i] >> slash >> normalIndex[i];
                vertexIndices.push_back(vertexIndex[i]);
                uvIndices.push_back(uvIndex[i]);
                normalIndices.push_back(normalIndex[i]);
            }
        }
    }

    // Populate the global vectors with the loaded data
    for (unsigned int i = 0; i < vertexIndices.size(); i++) {
        unsigned int vertexIndex = vertexIndices[i];
        unsigned int uvIndex = uvIndices[i];
        unsigned int normalIndex = normalIndices[i];

        vertices.push_back(temp_vertices[(vertexIndex - 1) * 3 + 0]);
        vertices.push_back(temp_vertices[(vertexIndex - 1) * 3 + 1]);
        vertices.push_back(temp_vertices[(vertexIndex - 1) * 3 + 2]);

        uvs.push_back(temp_uvs[(uvIndex - 1) * 2 + 0]);
        uvs.push_back(temp_uvs[(uvIndex - 1) * 2 + 1]);

        normals.push_back(temp_normals[(normalIndex - 1) * 3 + 0]);
        normals.push_back(temp_normals[(normalIndex - 1) * 3 + 1]);
        normals.push_back(temp_normals[(normalIndex - 1) * 3 + 2]);
    }

    std::cerr << "Loaded " << vertices.size() / 3 << " vertices" << std::endl;
    std::cerr << "Loaded " << uvs.size() / 2 << " texture coordinates" << std::endl;
    std::cerr << "Loaded " << normals.size() / 3 << " normals" << std::endl;

    return true;
}

// Function to load a BMP texture
GLuint LoadBMP(const char* imagepath) {
    unsigned char header[54];
    unsigned int dataPos;
    unsigned int width, height;

    FILE* file = fopen(imagepath, "rb");
    if (!file) {
        std::cerr << "Image could not be opened: " << imagepath << std::endl;
        return 0;
    }

    if (fread(header, 1, 54, file) != 54 || header[0] != 'B' || header[1] != 'M') {
        std::cerr << "Not a correct BMP file: " << imagepath << std::endl;
        fclose(file);
        return 0;
    }

    dataPos = *(int*)&(header[0x0A]);
    unsigned int imageSize = *(int*)&(header[0x22]);
    width = *(int*)&(header[0x12]);
    height = *(int*)&(header[0x16]);

    if (imageSize == 0) imageSize = width * height * 3; // Calculate image size if not provided
    if (dataPos == 0) dataPos = 54; // BMP header size

    // Use a vector to manage image data automatically
    std::vector<unsigned char> data(imageSize);
    fread(data.data(), sizeof(unsigned char), imageSize, file);
    fclose(file);

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    // Upload texture data to OpenGL
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data.data());
    
    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    std::cerr << "Loaded BMP texture: " << imagepath << std::endl;
    
    return textureID; // Return the generated texture ID
}

// Function to render the loaded model
void RenderModel() {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Scale the model (adjust as needed)
    glPushMatrix();
    glScalef(1.0f, 1.0f, 1.0f); 

    glBegin(GL_TRIANGLES); // Start drawing triangles
    for (size_t i = 0; i < vertices.size() / 3; i++) {
        if (!normals.empty()) {
            glNormal3f(normals[3 * i + 0], normals[3 * i + 1], normals[3 * i + 2]);
        }
        
        if (!uvs.empty()) {
            glTexCoord2f(uvs[2 * i + 0], uvs[2 * i + 1]);
        }
        
        glVertex3f(vertices[3 * i + 0], vertices[3 * i + 1], vertices[3 * i + 2]);
    }
    
    glEnd(); // End drawing

    glPopMatrix();
    
    glDisable(GL_TEXTURE_2D); // Disable texturing
}