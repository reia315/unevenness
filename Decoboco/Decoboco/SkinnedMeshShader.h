#pragma once
#include "Color.h"
#include "Vector3.h"
#include "Matrix4.h"
#include "SkinnedMesh.h"

class SkinnedMeshShader
	: public SkinnedMesh::Shader
{
public:

	struct Lignt
	{
		Color ambient;
		Color diffuse;
		Color specular;
		Vector3 position;
	};

public:

	SkinnedMeshShader(ID3D11Device* device, ID3D11DeviceContext* deviceContext);

	~SkinnedMeshShader();

	void load(const std::string& fileName);

	void world(const Matrix4& mat);

	void view(const Matrix4& mat);

	void projection(const Matrix4& mat);

	void light(const SkinnedMeshShader::Lignt& light);

	void clear();

	virtual void begin() override;

	virtual void end() override;

	virtual void material(const Mesh::Material& material) override;

	virtual void boneMatrices(int size, const Matrix4 matrices[]) override;

	SkinnedMeshShader(const SkinnedMeshShader& other) = delete;
	SkinnedMeshShader& operator = (const SkinnedMeshShader& other) = delete;

private:

	ID3D11Device* m_device{ nullptr };
	ID3D11DeviceContext* m_deviceContext{ nullptr };
	ID3D11Buffer* m_constantBuffer{ nullptr };
	ID3D11VertexShader* m_vertexShader{ nullptr };
	ID3D11PixelShader* m_pixelShader{ nullptr };
	ID3D11InputLayout* m_vertexLayout{ nullptr };

	struct BufferData
	{
		Matrix4 world;
		Matrix4 view;
		Matrix4 projection;
		Matrix4 boneMatrices[256];

		Color matAmbient;
		Color matDiffuse;
		Color matSpecular;
		Color matEmissive;
		float matShiniess;
		float pad0[3];

		Color lightAmbient;
		Color lightDiffuse;
		Color lightSpecular;
		Vector3 lightPosition;
		float pad11[1];
	};

	BufferData m_data;
};