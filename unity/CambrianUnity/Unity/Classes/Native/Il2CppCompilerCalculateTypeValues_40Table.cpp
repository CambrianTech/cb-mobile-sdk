#include "il2cpp-config.h"

#ifndef _MSC_VER
# include <alloca.h>
#else
# include <malloc.h>
#endif


#include <cstring>
#include <string.h>
#include <stdio.h>
#include <cmath>
#include <limits>
#include <assert.h>
#include <stdint.h>

#include "il2cpp-class-internals.h"
#include "codegen/il2cpp-codegen.h"
#include "il2cpp-object-internals.h"


// Shaw.Data.InstallationMethod
struct InstallationMethod_t610149799;
// Shaw.Data.Product
struct Product_t2706958693;
// Shaw.Data.ProductColor
struct ProductColor_t905712915;
// Shaw.Data.ProductVariation[]
struct ProductVariationU5BU5D_t63941910;
// Shaw.Data.SceneLocation
struct SceneLocation_t1733776779;
// System.Byte[]
struct ByteU5BU5D_t4116647657;
// System.Collections.Generic.HashSet`1<UnityEngine.RenderTexture>
struct HashSet_1_t673836907;
// System.Collections.Generic.List`1<UnityEngine.Texture2D>
struct List_1_t1017553631;
// System.Single[]
struct SingleU5BU5D_t1444911251;
// System.String
struct String_t;
// System.Void
struct Void_t1185182177;
// UnityEngine.GameObject
struct GameObject_t1113636619;
// UnityEngine.Renderer
struct Renderer_t2627027031;
// UnityEngine.Rigidbody
struct Rigidbody_t3916780224;
// UnityEngine.Transform
struct Transform_t3600365921;
// UnityEngine.Vector3[]
struct Vector3U5BU5D_t1718750761;




#ifndef RUNTIMEOBJECT_H
#define RUNTIMEOBJECT_H
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#pragma clang diagnostic ignored "-Wunused-variable"
#endif

// System.Object

#ifdef __clang__
#pragma clang diagnostic pop
#endif
#endif // RUNTIMEOBJECT_H
#ifndef UNIFORMS_T1233092826_H
#define UNIFORMS_T1233092826_H
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#pragma clang diagnostic ignored "-Wunused-variable"
#endif

// ExampleWheelController/Uniforms
struct  Uniforms_t1233092826  : public RuntimeObject
{
public:

public:
};

struct Uniforms_t1233092826_StaticFields
{
public:
	// System.Int32 ExampleWheelController/Uniforms::_MotionAmount
	int32_t ____MotionAmount_0;

public:
	inline static int32_t get_offset_of__MotionAmount_0() { return static_cast<int32_t>(offsetof(Uniforms_t1233092826_StaticFields, ____MotionAmount_0)); }
	inline int32_t get__MotionAmount_0() const { return ____MotionAmount_0; }
	inline int32_t* get_address_of__MotionAmount_0() { return &____MotionAmount_0; }
	inline void set__MotionAmount_0(int32_t value)
	{
		____MotionAmount_0 = value;
	}
};

#ifdef __clang__
#pragma clang diagnostic pop
#endif
#endif // UNIFORMS_T1233092826_H
#ifndef INSTALLATIONMETHOD_T610149799_H
#define INSTALLATIONMETHOD_T610149799_H
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#pragma clang diagnostic ignored "-Wunused-variable"
#endif

// Shaw.Data.InstallationMethod
struct  InstallationMethod_t610149799  : public RuntimeObject
{
public:
	// System.String Shaw.Data.InstallationMethod::name
	String_t* ___name_0;

public:
	inline static int32_t get_offset_of_name_0() { return static_cast<int32_t>(offsetof(InstallationMethod_t610149799, ___name_0)); }
	inline String_t* get_name_0() const { return ___name_0; }
	inline String_t** get_address_of_name_0() { return &___name_0; }
	inline void set_name_0(String_t* value)
	{
		___name_0 = value;
		Il2CppCodeGenWriteBarrier((&___name_0), value);
	}
};

#ifdef __clang__
#pragma clang diagnostic pop
#endif
#endif // INSTALLATIONMETHOD_T610149799_H
#ifndef PRODUCT_T2706958693_H
#define PRODUCT_T2706958693_H
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#pragma clang diagnostic ignored "-Wunused-variable"
#endif

// Shaw.Data.Product
struct  Product_t2706958693  : public RuntimeObject
{
public:
	// System.String Shaw.Data.Product::name
	String_t* ___name_0;
	// System.Single Shaw.Data.Product::dpcm
	float ___dpcm_1;

public:
	inline static int32_t get_offset_of_name_0() { return static_cast<int32_t>(offsetof(Product_t2706958693, ___name_0)); }
	inline String_t* get_name_0() const { return ___name_0; }
	inline String_t** get_address_of_name_0() { return &___name_0; }
	inline void set_name_0(String_t* value)
	{
		___name_0 = value;
		Il2CppCodeGenWriteBarrier((&___name_0), value);
	}

	inline static int32_t get_offset_of_dpcm_1() { return static_cast<int32_t>(offsetof(Product_t2706958693, ___dpcm_1)); }
	inline float get_dpcm_1() const { return ___dpcm_1; }
	inline float* get_address_of_dpcm_1() { return &___dpcm_1; }
	inline void set_dpcm_1(float value)
	{
		___dpcm_1 = value;
	}
};

#ifdef __clang__
#pragma clang diagnostic pop
#endif
#endif // PRODUCT_T2706958693_H
#ifndef PRODUCTCOLOR_T905712915_H
#define PRODUCTCOLOR_T905712915_H
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#pragma clang diagnostic ignored "-Wunused-variable"
#endif

// Shaw.Data.ProductColor
struct  ProductColor_t905712915  : public RuntimeObject
{
public:
	// System.String Shaw.Data.ProductColor::name
	String_t* ___name_0;
	// Shaw.Data.ProductVariation[] Shaw.Data.ProductColor::variations
	ProductVariationU5BU5D_t63941910* ___variations_1;

public:
	inline static int32_t get_offset_of_name_0() { return static_cast<int32_t>(offsetof(ProductColor_t905712915, ___name_0)); }
	inline String_t* get_name_0() const { return ___name_0; }
	inline String_t** get_address_of_name_0() { return &___name_0; }
	inline void set_name_0(String_t* value)
	{
		___name_0 = value;
		Il2CppCodeGenWriteBarrier((&___name_0), value);
	}

	inline static int32_t get_offset_of_variations_1() { return static_cast<int32_t>(offsetof(ProductColor_t905712915, ___variations_1)); }
	inline ProductVariationU5BU5D_t63941910* get_variations_1() const { return ___variations_1; }
	inline ProductVariationU5BU5D_t63941910** get_address_of_variations_1() { return &___variations_1; }
	inline void set_variations_1(ProductVariationU5BU5D_t63941910* value)
	{
		___variations_1 = value;
		Il2CppCodeGenWriteBarrier((&___variations_1), value);
	}
};

#ifdef __clang__
#pragma clang diagnostic pop
#endif
#endif // PRODUCTCOLOR_T905712915_H
#ifndef PRODUCTVARIATION_T1816091343_H
#define PRODUCTVARIATION_T1816091343_H
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#pragma clang diagnostic ignored "-Wunused-variable"
#endif

// Shaw.Data.ProductVariation
struct  ProductVariation_t1816091343  : public RuntimeObject
{
public:
	// System.String Shaw.Data.ProductVariation::diffusePath
	String_t* ___diffusePath_0;
	// System.String Shaw.Data.ProductVariation::normalsPath
	String_t* ___normalsPath_1;
	// System.String Shaw.Data.ProductVariation::roughnessPath
	String_t* ___roughnessPath_2;
	// System.Byte[] Shaw.Data.ProductVariation::_diffuse
	ByteU5BU5D_t4116647657* ____diffuse_3;
	// System.Byte[] Shaw.Data.ProductVariation::_normal
	ByteU5BU5D_t4116647657* ____normal_4;
	// System.Byte[] Shaw.Data.ProductVariation::_roughness
	ByteU5BU5D_t4116647657* ____roughness_5;

public:
	inline static int32_t get_offset_of_diffusePath_0() { return static_cast<int32_t>(offsetof(ProductVariation_t1816091343, ___diffusePath_0)); }
	inline String_t* get_diffusePath_0() const { return ___diffusePath_0; }
	inline String_t** get_address_of_diffusePath_0() { return &___diffusePath_0; }
	inline void set_diffusePath_0(String_t* value)
	{
		___diffusePath_0 = value;
		Il2CppCodeGenWriteBarrier((&___diffusePath_0), value);
	}

	inline static int32_t get_offset_of_normalsPath_1() { return static_cast<int32_t>(offsetof(ProductVariation_t1816091343, ___normalsPath_1)); }
	inline String_t* get_normalsPath_1() const { return ___normalsPath_1; }
	inline String_t** get_address_of_normalsPath_1() { return &___normalsPath_1; }
	inline void set_normalsPath_1(String_t* value)
	{
		___normalsPath_1 = value;
		Il2CppCodeGenWriteBarrier((&___normalsPath_1), value);
	}

	inline static int32_t get_offset_of_roughnessPath_2() { return static_cast<int32_t>(offsetof(ProductVariation_t1816091343, ___roughnessPath_2)); }
	inline String_t* get_roughnessPath_2() const { return ___roughnessPath_2; }
	inline String_t** get_address_of_roughnessPath_2() { return &___roughnessPath_2; }
	inline void set_roughnessPath_2(String_t* value)
	{
		___roughnessPath_2 = value;
		Il2CppCodeGenWriteBarrier((&___roughnessPath_2), value);
	}

	inline static int32_t get_offset_of__diffuse_3() { return static_cast<int32_t>(offsetof(ProductVariation_t1816091343, ____diffuse_3)); }
	inline ByteU5BU5D_t4116647657* get__diffuse_3() const { return ____diffuse_3; }
	inline ByteU5BU5D_t4116647657** get_address_of__diffuse_3() { return &____diffuse_3; }
	inline void set__diffuse_3(ByteU5BU5D_t4116647657* value)
	{
		____diffuse_3 = value;
		Il2CppCodeGenWriteBarrier((&____diffuse_3), value);
	}

	inline static int32_t get_offset_of__normal_4() { return static_cast<int32_t>(offsetof(ProductVariation_t1816091343, ____normal_4)); }
	inline ByteU5BU5D_t4116647657* get__normal_4() const { return ____normal_4; }
	inline ByteU5BU5D_t4116647657** get_address_of__normal_4() { return &____normal_4; }
	inline void set__normal_4(ByteU5BU5D_t4116647657* value)
	{
		____normal_4 = value;
		Il2CppCodeGenWriteBarrier((&____normal_4), value);
	}

	inline static int32_t get_offset_of__roughness_5() { return static_cast<int32_t>(offsetof(ProductVariation_t1816091343, ____roughness_5)); }
	inline ByteU5BU5D_t4116647657* get__roughness_5() const { return ____roughness_5; }
	inline ByteU5BU5D_t4116647657** get_address_of__roughness_5() { return &____roughness_5; }
	inline void set__roughness_5(ByteU5BU5D_t4116647657* value)
	{
		____roughness_5 = value;
		Il2CppCodeGenWriteBarrier((&____roughness_5), value);
	}
};

#ifdef __clang__
#pragma clang diagnostic pop
#endif
#endif // PRODUCTVARIATION_T1816091343_H
#ifndef SCENELOCATION_T1733776779_H
#define SCENELOCATION_T1733776779_H
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#pragma clang diagnostic ignored "-Wunused-variable"
#endif

// Shaw.Data.SceneLocation
struct  SceneLocation_t1733776779  : public RuntimeObject
{
public:
	// System.String Shaw.Data.SceneLocation::name
	String_t* ___name_0;
	// System.Single[] Shaw.Data.SceneLocation::worldPosition
	SingleU5BU5D_t1444911251* ___worldPosition_1;

public:
	inline static int32_t get_offset_of_name_0() { return static_cast<int32_t>(offsetof(SceneLocation_t1733776779, ___name_0)); }
	inline String_t* get_name_0() const { return ___name_0; }
	inline String_t** get_address_of_name_0() { return &___name_0; }
	inline void set_name_0(String_t* value)
	{
		___name_0 = value;
		Il2CppCodeGenWriteBarrier((&___name_0), value);
	}

	inline static int32_t get_offset_of_worldPosition_1() { return static_cast<int32_t>(offsetof(SceneLocation_t1733776779, ___worldPosition_1)); }
	inline SingleU5BU5D_t1444911251* get_worldPosition_1() const { return ___worldPosition_1; }
	inline SingleU5BU5D_t1444911251** get_address_of_worldPosition_1() { return &___worldPosition_1; }
	inline void set_worldPosition_1(SingleU5BU5D_t1444911251* value)
	{
		___worldPosition_1 = value;
		Il2CppCodeGenWriteBarrier((&___worldPosition_1), value);
	}
};

#ifdef __clang__
#pragma clang diagnostic pop
#endif
#endif // SCENELOCATION_T1733776779_H
#ifndef UPDATEINSTALLATIONCOMMAND_T3591009552_H
#define UPDATEINSTALLATIONCOMMAND_T3591009552_H
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#pragma clang diagnostic ignored "-Wunused-variable"
#endif

// Shaw.Data.UpdateInstallationCommand
struct  UpdateInstallationCommand_t3591009552  : public RuntimeObject
{
public:
	// Shaw.Data.Product Shaw.Data.UpdateInstallationCommand::product
	Product_t2706958693 * ___product_0;
	// Shaw.Data.InstallationMethod Shaw.Data.UpdateInstallationCommand::method
	InstallationMethod_t610149799 * ___method_1;

public:
	inline static int32_t get_offset_of_product_0() { return static_cast<int32_t>(offsetof(UpdateInstallationCommand_t3591009552, ___product_0)); }
	inline Product_t2706958693 * get_product_0() const { return ___product_0; }
	inline Product_t2706958693 ** get_address_of_product_0() { return &___product_0; }
	inline void set_product_0(Product_t2706958693 * value)
	{
		___product_0 = value;
		Il2CppCodeGenWriteBarrier((&___product_0), value);
	}

	inline static int32_t get_offset_of_method_1() { return static_cast<int32_t>(offsetof(UpdateInstallationCommand_t3591009552, ___method_1)); }
	inline InstallationMethod_t610149799 * get_method_1() const { return ___method_1; }
	inline InstallationMethod_t610149799 ** get_address_of_method_1() { return &___method_1; }
	inline void set_method_1(InstallationMethod_t610149799 * value)
	{
		___method_1 = value;
		Il2CppCodeGenWriteBarrier((&___method_1), value);
	}
};

#ifdef __clang__
#pragma clang diagnostic pop
#endif
#endif // UPDATEINSTALLATIONCOMMAND_T3591009552_H
#ifndef UPDATELOCATIONCOMMAND_T2892508292_H
#define UPDATELOCATIONCOMMAND_T2892508292_H
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#pragma clang diagnostic ignored "-Wunused-variable"
#endif

// Shaw.Data.UpdateLocationCommand
struct  UpdateLocationCommand_t2892508292  : public RuntimeObject
{
public:
	// Shaw.Data.SceneLocation Shaw.Data.UpdateLocationCommand::location
	SceneLocation_t1733776779 * ___location_0;

public:
	inline static int32_t get_offset_of_location_0() { return static_cast<int32_t>(offsetof(UpdateLocationCommand_t2892508292, ___location_0)); }
	inline SceneLocation_t1733776779 * get_location_0() const { return ___location_0; }
	inline SceneLocation_t1733776779 ** get_address_of_location_0() { return &___location_0; }
	inline void set_location_0(SceneLocation_t1733776779 * value)
	{
		___location_0 = value;
		Il2CppCodeGenWriteBarrier((&___location_0), value);
	}
};

#ifdef __clang__
#pragma clang diagnostic pop
#endif
#endif // UPDATELOCATIONCOMMAND_T2892508292_H
#ifndef UPDATEMATERIALCOMMAND_T1067309048_H
#define UPDATEMATERIALCOMMAND_T1067309048_H
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#pragma clang diagnostic ignored "-Wunused-variable"
#endif

// Shaw.Data.UpdateMaterialCommand
struct  UpdateMaterialCommand_t1067309048  : public RuntimeObject
{
public:
	// Shaw.Data.Product Shaw.Data.UpdateMaterialCommand::product
	Product_t2706958693 * ___product_0;
	// Shaw.Data.ProductColor Shaw.Data.UpdateMaterialCommand::color
	ProductColor_t905712915 * ___color_1;

public:
	inline static int32_t get_offset_of_product_0() { return static_cast<int32_t>(offsetof(UpdateMaterialCommand_t1067309048, ___product_0)); }
	inline Product_t2706958693 * get_product_0() const { return ___product_0; }
	inline Product_t2706958693 ** get_address_of_product_0() { return &___product_0; }
	inline void set_product_0(Product_t2706958693 * value)
	{
		___product_0 = value;
		Il2CppCodeGenWriteBarrier((&___product_0), value);
	}

	inline static int32_t get_offset_of_color_1() { return static_cast<int32_t>(offsetof(UpdateMaterialCommand_t1067309048, ___color_1)); }
	inline ProductColor_t905712915 * get_color_1() const { return ___color_1; }
	inline ProductColor_t905712915 ** get_address_of_color_1() { return &___color_1; }
	inline void set_color_1(ProductColor_t905712915 * value)
	{
		___color_1 = value;
		Il2CppCodeGenWriteBarrier((&___color_1), value);
	}
};

#ifdef __clang__
#pragma clang diagnostic pop
#endif
#endif // UPDATEMATERIALCOMMAND_T1067309048_H
#ifndef UTILITY_T1436046025_H
#define UTILITY_T1436046025_H
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#pragma clang diagnostic ignored "-Wunused-variable"
#endif

// Shaw.Data.Utility
struct  Utility_t1436046025  : public RuntimeObject
{
public:

public:
};

#ifdef __clang__
#pragma clang diagnostic pop
#endif
#endif // UTILITY_T1436046025_H
#ifndef VALUETYPE_T3640485471_H
#define VALUETYPE_T3640485471_H
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#pragma clang diagnostic ignored "-Wunused-variable"
#endif

// System.ValueType
struct  ValueType_t3640485471  : public RuntimeObject
{
public:

public:
};

#ifdef __clang__
#pragma clang diagnostic pop
#endif
// Native definition for P/Invoke marshalling of System.ValueType
struct ValueType_t3640485471_marshaled_pinvoke
{
};
// Native definition for COM marshalling of System.ValueType
struct ValueType_t3640485471_marshaled_com
{
};
#endif // VALUETYPE_T3640485471_H
#ifndef RENDERTEXTUREFACTORY_T1946967824_H
#define RENDERTEXTUREFACTORY_T1946967824_H
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#pragma clang diagnostic ignored "-Wunused-variable"
#endif

// UnityEngine.PostProcessing.RenderTextureFactory
struct  RenderTextureFactory_t1946967824  : public RuntimeObject
{
public:
	// System.Collections.Generic.HashSet`1<UnityEngine.RenderTexture> UnityEngine.PostProcessing.RenderTextureFactory::m_TemporaryRTs
	HashSet_1_t673836907 * ___m_TemporaryRTs_0;

public:
	inline static int32_t get_offset_of_m_TemporaryRTs_0() { return static_cast<int32_t>(offsetof(RenderTextureFactory_t1946967824, ___m_TemporaryRTs_0)); }
	inline HashSet_1_t673836907 * get_m_TemporaryRTs_0() const { return ___m_TemporaryRTs_0; }
	inline HashSet_1_t673836907 ** get_address_of_m_TemporaryRTs_0() { return &___m_TemporaryRTs_0; }
	inline void set_m_TemporaryRTs_0(HashSet_1_t673836907 * value)
	{
		___m_TemporaryRTs_0 = value;
		Il2CppCodeGenWriteBarrier((&___m_TemporaryRTs_0), value);
	}
};

#ifdef __clang__
#pragma clang diagnostic pop
#endif
#endif // RENDERTEXTUREFACTORY_T1946967824_H
#ifndef U24ARRAYTYPEU3D12_T2488454197_H
#define U24ARRAYTYPEU3D12_T2488454197_H
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#pragma clang diagnostic ignored "-Wunused-variable"
#endif

// <PrivateImplementationDetails>/$ArrayType=12
#pragma pack(push, tp, 1)
struct  U24ArrayTypeU3D12_t2488454197 
{
public:
	union
	{
		struct
		{
		};
		uint8_t U24ArrayTypeU3D12_t2488454197__padding[12];
	};

public:
};
#pragma pack(pop, tp)

#ifdef __clang__
#pragma clang diagnostic pop
#endif
#endif // U24ARRAYTYPEU3D12_T2488454197_H
#ifndef U24ARRAYTYPEU3D24_T2467506693_H
#define U24ARRAYTYPEU3D24_T2467506693_H
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#pragma clang diagnostic ignored "-Wunused-variable"
#endif

// <PrivateImplementationDetails>/$ArrayType=24
#pragma pack(push, tp, 1)
struct  U24ArrayTypeU3D24_t2467506693 
{
public:
	union
	{
		struct
		{
		};
		uint8_t U24ArrayTypeU3D24_t2467506693__padding[24];
	};

public:
};
#pragma pack(pop, tp)

#ifdef __clang__
#pragma clang diagnostic pop
#endif
#endif // U24ARRAYTYPEU3D24_T2467506693_H
#ifndef INTPTR_T_H
#define INTPTR_T_H
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#pragma clang diagnostic ignored "-Wunused-variable"
#endif

// System.IntPtr
struct  IntPtr_t 
{
public:
	// System.Void* System.IntPtr::m_value
	void* ___m_value_0;

public:
	inline static int32_t get_offset_of_m_value_0() { return static_cast<int32_t>(offsetof(IntPtr_t, ___m_value_0)); }
	inline void* get_m_value_0() const { return ___m_value_0; }
	inline void** get_address_of_m_value_0() { return &___m_value_0; }
	inline void set_m_value_0(void* value)
	{
		___m_value_0 = value;
	}
};

struct IntPtr_t_StaticFields
{
public:
	// System.IntPtr System.IntPtr::Zero
	intptr_t ___Zero_1;

public:
	inline static int32_t get_offset_of_Zero_1() { return static_cast<int32_t>(offsetof(IntPtr_t_StaticFields, ___Zero_1)); }
	inline intptr_t get_Zero_1() const { return ___Zero_1; }
	inline intptr_t* get_address_of_Zero_1() { return &___Zero_1; }
	inline void set_Zero_1(intptr_t value)
	{
		___Zero_1 = value;
	}
};

#ifdef __clang__
#pragma clang diagnostic pop
#endif
#endif // INTPTR_T_H
#ifndef QUATERNION_T2301928331_H
#define QUATERNION_T2301928331_H
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#pragma clang diagnostic ignored "-Wunused-variable"
#endif

// UnityEngine.Quaternion
struct  Quaternion_t2301928331 
{
public:
	// System.Single UnityEngine.Quaternion::x
	float ___x_0;
	// System.Single UnityEngine.Quaternion::y
	float ___y_1;
	// System.Single UnityEngine.Quaternion::z
	float ___z_2;
	// System.Single UnityEngine.Quaternion::w
	float ___w_3;

public:
	inline static int32_t get_offset_of_x_0() { return static_cast<int32_t>(offsetof(Quaternion_t2301928331, ___x_0)); }
	inline float get_x_0() const { return ___x_0; }
	inline float* get_address_of_x_0() { return &___x_0; }
	inline void set_x_0(float value)
	{
		___x_0 = value;
	}

	inline static int32_t get_offset_of_y_1() { return static_cast<int32_t>(offsetof(Quaternion_t2301928331, ___y_1)); }
	inline float get_y_1() const { return ___y_1; }
	inline float* get_address_of_y_1() { return &___y_1; }
	inline void set_y_1(float value)
	{
		___y_1 = value;
	}

	inline static int32_t get_offset_of_z_2() { return static_cast<int32_t>(offsetof(Quaternion_t2301928331, ___z_2)); }
	inline float get_z_2() const { return ___z_2; }
	inline float* get_address_of_z_2() { return &___z_2; }
	inline void set_z_2(float value)
	{
		___z_2 = value;
	}

	inline static int32_t get_offset_of_w_3() { return static_cast<int32_t>(offsetof(Quaternion_t2301928331, ___w_3)); }
	inline float get_w_3() const { return ___w_3; }
	inline float* get_address_of_w_3() { return &___w_3; }
	inline void set_w_3(float value)
	{
		___w_3 = value;
	}
};

struct Quaternion_t2301928331_StaticFields
{
public:
	// UnityEngine.Quaternion UnityEngine.Quaternion::identityQuaternion
	Quaternion_t2301928331  ___identityQuaternion_4;

public:
	inline static int32_t get_offset_of_identityQuaternion_4() { return static_cast<int32_t>(offsetof(Quaternion_t2301928331_StaticFields, ___identityQuaternion_4)); }
	inline Quaternion_t2301928331  get_identityQuaternion_4() const { return ___identityQuaternion_4; }
	inline Quaternion_t2301928331 * get_address_of_identityQuaternion_4() { return &___identityQuaternion_4; }
	inline void set_identityQuaternion_4(Quaternion_t2301928331  value)
	{
		___identityQuaternion_4 = value;
	}
};

#ifdef __clang__
#pragma clang diagnostic pop
#endif
#endif // QUATERNION_T2301928331_H
#ifndef VECTOR2_T2156229523_H
#define VECTOR2_T2156229523_H
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#pragma clang diagnostic ignored "-Wunused-variable"
#endif

// UnityEngine.Vector2
struct  Vector2_t2156229523 
{
public:
	// System.Single UnityEngine.Vector2::x
	float ___x_0;
	// System.Single UnityEngine.Vector2::y
	float ___y_1;

public:
	inline static int32_t get_offset_of_x_0() { return static_cast<int32_t>(offsetof(Vector2_t2156229523, ___x_0)); }
	inline float get_x_0() const { return ___x_0; }
	inline float* get_address_of_x_0() { return &___x_0; }
	inline void set_x_0(float value)
	{
		___x_0 = value;
	}

	inline static int32_t get_offset_of_y_1() { return static_cast<int32_t>(offsetof(Vector2_t2156229523, ___y_1)); }
	inline float get_y_1() const { return ___y_1; }
	inline float* get_address_of_y_1() { return &___y_1; }
	inline void set_y_1(float value)
	{
		___y_1 = value;
	}
};

struct Vector2_t2156229523_StaticFields
{
public:
	// UnityEngine.Vector2 UnityEngine.Vector2::zeroVector
	Vector2_t2156229523  ___zeroVector_2;
	// UnityEngine.Vector2 UnityEngine.Vector2::oneVector
	Vector2_t2156229523  ___oneVector_3;
	// UnityEngine.Vector2 UnityEngine.Vector2::upVector
	Vector2_t2156229523  ___upVector_4;
	// UnityEngine.Vector2 UnityEngine.Vector2::downVector
	Vector2_t2156229523  ___downVector_5;
	// UnityEngine.Vector2 UnityEngine.Vector2::leftVector
	Vector2_t2156229523  ___leftVector_6;
	// UnityEngine.Vector2 UnityEngine.Vector2::rightVector
	Vector2_t2156229523  ___rightVector_7;
	// UnityEngine.Vector2 UnityEngine.Vector2::positiveInfinityVector
	Vector2_t2156229523  ___positiveInfinityVector_8;
	// UnityEngine.Vector2 UnityEngine.Vector2::negativeInfinityVector
	Vector2_t2156229523  ___negativeInfinityVector_9;

public:
	inline static int32_t get_offset_of_zeroVector_2() { return static_cast<int32_t>(offsetof(Vector2_t2156229523_StaticFields, ___zeroVector_2)); }
	inline Vector2_t2156229523  get_zeroVector_2() const { return ___zeroVector_2; }
	inline Vector2_t2156229523 * get_address_of_zeroVector_2() { return &___zeroVector_2; }
	inline void set_zeroVector_2(Vector2_t2156229523  value)
	{
		___zeroVector_2 = value;
	}

	inline static int32_t get_offset_of_oneVector_3() { return static_cast<int32_t>(offsetof(Vector2_t2156229523_StaticFields, ___oneVector_3)); }
	inline Vector2_t2156229523  get_oneVector_3() const { return ___oneVector_3; }
	inline Vector2_t2156229523 * get_address_of_oneVector_3() { return &___oneVector_3; }
	inline void set_oneVector_3(Vector2_t2156229523  value)
	{
		___oneVector_3 = value;
	}

	inline static int32_t get_offset_of_upVector_4() { return static_cast<int32_t>(offsetof(Vector2_t2156229523_StaticFields, ___upVector_4)); }
	inline Vector2_t2156229523  get_upVector_4() const { return ___upVector_4; }
	inline Vector2_t2156229523 * get_address_of_upVector_4() { return &___upVector_4; }
	inline void set_upVector_4(Vector2_t2156229523  value)
	{
		___upVector_4 = value;
	}

	inline static int32_t get_offset_of_downVector_5() { return static_cast<int32_t>(offsetof(Vector2_t2156229523_StaticFields, ___downVector_5)); }
	inline Vector2_t2156229523  get_downVector_5() const { return ___downVector_5; }
	inline Vector2_t2156229523 * get_address_of_downVector_5() { return &___downVector_5; }
	inline void set_downVector_5(Vector2_t2156229523  value)
	{
		___downVector_5 = value;
	}

	inline static int32_t get_offset_of_leftVector_6() { return static_cast<int32_t>(offsetof(Vector2_t2156229523_StaticFields, ___leftVector_6)); }
	inline Vector2_t2156229523  get_leftVector_6() const { return ___leftVector_6; }
	inline Vector2_t2156229523 * get_address_of_leftVector_6() { return &___leftVector_6; }
	inline void set_leftVector_6(Vector2_t2156229523  value)
	{
		___leftVector_6 = value;
	}

	inline static int32_t get_offset_of_rightVector_7() { return static_cast<int32_t>(offsetof(Vector2_t2156229523_StaticFields, ___rightVector_7)); }
	inline Vector2_t2156229523  get_rightVector_7() const { return ___rightVector_7; }
	inline Vector2_t2156229523 * get_address_of_rightVector_7() { return &___rightVector_7; }
	inline void set_rightVector_7(Vector2_t2156229523  value)
	{
		___rightVector_7 = value;
	}

	inline static int32_t get_offset_of_positiveInfinityVector_8() { return static_cast<int32_t>(offsetof(Vector2_t2156229523_StaticFields, ___positiveInfinityVector_8)); }
	inline Vector2_t2156229523  get_positiveInfinityVector_8() const { return ___positiveInfinityVector_8; }
	inline Vector2_t2156229523 * get_address_of_positiveInfinityVector_8() { return &___positiveInfinityVector_8; }
	inline void set_positiveInfinityVector_8(Vector2_t2156229523  value)
	{
		___positiveInfinityVector_8 = value;
	}

	inline static int32_t get_offset_of_negativeInfinityVector_9() { return static_cast<int32_t>(offsetof(Vector2_t2156229523_StaticFields, ___negativeInfinityVector_9)); }
	inline Vector2_t2156229523  get_negativeInfinityVector_9() const { return ___negativeInfinityVector_9; }
	inline Vector2_t2156229523 * get_address_of_negativeInfinityVector_9() { return &___negativeInfinityVector_9; }
	inline void set_negativeInfinityVector_9(Vector2_t2156229523  value)
	{
		___negativeInfinityVector_9 = value;
	}
};

#ifdef __clang__
#pragma clang diagnostic pop
#endif
#endif // VECTOR2_T2156229523_H
#ifndef VECTOR3_T3722313464_H
#define VECTOR3_T3722313464_H
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#pragma clang diagnostic ignored "-Wunused-variable"
#endif

// UnityEngine.Vector3
struct  Vector3_t3722313464 
{
public:
	// System.Single UnityEngine.Vector3::x
	float ___x_2;
	// System.Single UnityEngine.Vector3::y
	float ___y_3;
	// System.Single UnityEngine.Vector3::z
	float ___z_4;

public:
	inline static int32_t get_offset_of_x_2() { return static_cast<int32_t>(offsetof(Vector3_t3722313464, ___x_2)); }
	inline float get_x_2() const { return ___x_2; }
	inline float* get_address_of_x_2() { return &___x_2; }
	inline void set_x_2(float value)
	{
		___x_2 = value;
	}

	inline static int32_t get_offset_of_y_3() { return static_cast<int32_t>(offsetof(Vector3_t3722313464, ___y_3)); }
	inline float get_y_3() const { return ___y_3; }
	inline float* get_address_of_y_3() { return &___y_3; }
	inline void set_y_3(float value)
	{
		___y_3 = value;
	}

	inline static int32_t get_offset_of_z_4() { return static_cast<int32_t>(offsetof(Vector3_t3722313464, ___z_4)); }
	inline float get_z_4() const { return ___z_4; }
	inline float* get_address_of_z_4() { return &___z_4; }
	inline void set_z_4(float value)
	{
		___z_4 = value;
	}
};

struct Vector3_t3722313464_StaticFields
{
public:
	// UnityEngine.Vector3 UnityEngine.Vector3::zeroVector
	Vector3_t3722313464  ___zeroVector_5;
	// UnityEngine.Vector3 UnityEngine.Vector3::oneVector
	Vector3_t3722313464  ___oneVector_6;
	// UnityEngine.Vector3 UnityEngine.Vector3::upVector
	Vector3_t3722313464  ___upVector_7;
	// UnityEngine.Vector3 UnityEngine.Vector3::downVector
	Vector3_t3722313464  ___downVector_8;
	// UnityEngine.Vector3 UnityEngine.Vector3::leftVector
	Vector3_t3722313464  ___leftVector_9;
	// UnityEngine.Vector3 UnityEngine.Vector3::rightVector
	Vector3_t3722313464  ___rightVector_10;
	// UnityEngine.Vector3 UnityEngine.Vector3::forwardVector
	Vector3_t3722313464  ___forwardVector_11;
	// UnityEngine.Vector3 UnityEngine.Vector3::backVector
	Vector3_t3722313464  ___backVector_12;
	// UnityEngine.Vector3 UnityEngine.Vector3::positiveInfinityVector
	Vector3_t3722313464  ___positiveInfinityVector_13;
	// UnityEngine.Vector3 UnityEngine.Vector3::negativeInfinityVector
	Vector3_t3722313464  ___negativeInfinityVector_14;

public:
	inline static int32_t get_offset_of_zeroVector_5() { return static_cast<int32_t>(offsetof(Vector3_t3722313464_StaticFields, ___zeroVector_5)); }
	inline Vector3_t3722313464  get_zeroVector_5() const { return ___zeroVector_5; }
	inline Vector3_t3722313464 * get_address_of_zeroVector_5() { return &___zeroVector_5; }
	inline void set_zeroVector_5(Vector3_t3722313464  value)
	{
		___zeroVector_5 = value;
	}

	inline static int32_t get_offset_of_oneVector_6() { return static_cast<int32_t>(offsetof(Vector3_t3722313464_StaticFields, ___oneVector_6)); }
	inline Vector3_t3722313464  get_oneVector_6() const { return ___oneVector_6; }
	inline Vector3_t3722313464 * get_address_of_oneVector_6() { return &___oneVector_6; }
	inline void set_oneVector_6(Vector3_t3722313464  value)
	{
		___oneVector_6 = value;
	}

	inline static int32_t get_offset_of_upVector_7() { return static_cast<int32_t>(offsetof(Vector3_t3722313464_StaticFields, ___upVector_7)); }
	inline Vector3_t3722313464  get_upVector_7() const { return ___upVector_7; }
	inline Vector3_t3722313464 * get_address_of_upVector_7() { return &___upVector_7; }
	inline void set_upVector_7(Vector3_t3722313464  value)
	{
		___upVector_7 = value;
	}

	inline static int32_t get_offset_of_downVector_8() { return static_cast<int32_t>(offsetof(Vector3_t3722313464_StaticFields, ___downVector_8)); }
	inline Vector3_t3722313464  get_downVector_8() const { return ___downVector_8; }
	inline Vector3_t3722313464 * get_address_of_downVector_8() { return &___downVector_8; }
	inline void set_downVector_8(Vector3_t3722313464  value)
	{
		___downVector_8 = value;
	}

	inline static int32_t get_offset_of_leftVector_9() { return static_cast<int32_t>(offsetof(Vector3_t3722313464_StaticFields, ___leftVector_9)); }
	inline Vector3_t3722313464  get_leftVector_9() const { return ___leftVector_9; }
	inline Vector3_t3722313464 * get_address_of_leftVector_9() { return &___leftVector_9; }
	inline void set_leftVector_9(Vector3_t3722313464  value)
	{
		___leftVector_9 = value;
	}

	inline static int32_t get_offset_of_rightVector_10() { return static_cast<int32_t>(offsetof(Vector3_t3722313464_StaticFields, ___rightVector_10)); }
	inline Vector3_t3722313464  get_rightVector_10() const { return ___rightVector_10; }
	inline Vector3_t3722313464 * get_address_of_rightVector_10() { return &___rightVector_10; }
	inline void set_rightVector_10(Vector3_t3722313464  value)
	{
		___rightVector_10 = value;
	}

	inline static int32_t get_offset_of_forwardVector_11() { return static_cast<int32_t>(offsetof(Vector3_t3722313464_StaticFields, ___forwardVector_11)); }
	inline Vector3_t3722313464  get_forwardVector_11() const { return ___forwardVector_11; }
	inline Vector3_t3722313464 * get_address_of_forwardVector_11() { return &___forwardVector_11; }
	inline void set_forwardVector_11(Vector3_t3722313464  value)
	{
		___forwardVector_11 = value;
	}

	inline static int32_t get_offset_of_backVector_12() { return static_cast<int32_t>(offsetof(Vector3_t3722313464_StaticFields, ___backVector_12)); }
	inline Vector3_t3722313464  get_backVector_12() const { return ___backVector_12; }
	inline Vector3_t3722313464 * get_address_of_backVector_12() { return &___backVector_12; }
	inline void set_backVector_12(Vector3_t3722313464  value)
	{
		___backVector_12 = value;
	}

	inline static int32_t get_offset_of_positiveInfinityVector_13() { return static_cast<int32_t>(offsetof(Vector3_t3722313464_StaticFields, ___positiveInfinityVector_13)); }
	inline Vector3_t3722313464  get_positiveInfinityVector_13() const { return ___positiveInfinityVector_13; }
	inline Vector3_t3722313464 * get_address_of_positiveInfinityVector_13() { return &___positiveInfinityVector_13; }
	inline void set_positiveInfinityVector_13(Vector3_t3722313464  value)
	{
		___positiveInfinityVector_13 = value;
	}

	inline static int32_t get_offset_of_negativeInfinityVector_14() { return static_cast<int32_t>(offsetof(Vector3_t3722313464_StaticFields, ___negativeInfinityVector_14)); }
	inline Vector3_t3722313464  get_negativeInfinityVector_14() const { return ___negativeInfinityVector_14; }
	inline Vector3_t3722313464 * get_address_of_negativeInfinityVector_14() { return &___negativeInfinityVector_14; }
	inline void set_negativeInfinityVector_14(Vector3_t3722313464  value)
	{
		___negativeInfinityVector_14 = value;
	}
};

#ifdef __clang__
#pragma clang diagnostic pop
#endif
#endif // VECTOR3_T3722313464_H
#ifndef U3CPRIVATEIMPLEMENTATIONDETAILSU3E_T3057255368_H
#define U3CPRIVATEIMPLEMENTATIONDETAILSU3E_T3057255368_H
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#pragma clang diagnostic ignored "-Wunused-variable"
#endif

// <PrivateImplementationDetails>
struct  U3CPrivateImplementationDetailsU3E_t3057255368  : public RuntimeObject
{
public:

public:
};

struct U3CPrivateImplementationDetailsU3E_t3057255368_StaticFields
{
public:
	// <PrivateImplementationDetails>/$ArrayType=12 <PrivateImplementationDetails>::$field-51A7A390CD6DE245186881400B18C9D822EFE240
	U24ArrayTypeU3D12_t2488454197  ___U24fieldU2D51A7A390CD6DE245186881400B18C9D822EFE240_0;
	// <PrivateImplementationDetails>/$ArrayType=24 <PrivateImplementationDetails>::$field-C90F38A020811481753795774EB5AF353F414C59
	U24ArrayTypeU3D24_t2467506693  ___U24fieldU2DC90F38A020811481753795774EB5AF353F414C59_1;

public:
	inline static int32_t get_offset_of_U24fieldU2D51A7A390CD6DE245186881400B18C9D822EFE240_0() { return static_cast<int32_t>(offsetof(U3CPrivateImplementationDetailsU3E_t3057255368_StaticFields, ___U24fieldU2D51A7A390CD6DE245186881400B18C9D822EFE240_0)); }
	inline U24ArrayTypeU3D12_t2488454197  get_U24fieldU2D51A7A390CD6DE245186881400B18C9D822EFE240_0() const { return ___U24fieldU2D51A7A390CD6DE245186881400B18C9D822EFE240_0; }
	inline U24ArrayTypeU3D12_t2488454197 * get_address_of_U24fieldU2D51A7A390CD6DE245186881400B18C9D822EFE240_0() { return &___U24fieldU2D51A7A390CD6DE245186881400B18C9D822EFE240_0; }
	inline void set_U24fieldU2D51A7A390CD6DE245186881400B18C9D822EFE240_0(U24ArrayTypeU3D12_t2488454197  value)
	{
		___U24fieldU2D51A7A390CD6DE245186881400B18C9D822EFE240_0 = value;
	}

	inline static int32_t get_offset_of_U24fieldU2DC90F38A020811481753795774EB5AF353F414C59_1() { return static_cast<int32_t>(offsetof(U3CPrivateImplementationDetailsU3E_t3057255368_StaticFields, ___U24fieldU2DC90F38A020811481753795774EB5AF353F414C59_1)); }
	inline U24ArrayTypeU3D24_t2467506693  get_U24fieldU2DC90F38A020811481753795774EB5AF353F414C59_1() const { return ___U24fieldU2DC90F38A020811481753795774EB5AF353F414C59_1; }
	inline U24ArrayTypeU3D24_t2467506693 * get_address_of_U24fieldU2DC90F38A020811481753795774EB5AF353F414C59_1() { return &___U24fieldU2DC90F38A020811481753795774EB5AF353F414C59_1; }
	inline void set_U24fieldU2DC90F38A020811481753795774EB5AF353F414C59_1(U24ArrayTypeU3D24_t2467506693  value)
	{
		___U24fieldU2DC90F38A020811481753795774EB5AF353F414C59_1 = value;
	}
};

#ifdef __clang__
#pragma clang diagnostic pop
#endif
#endif // U3CPRIVATEIMPLEMENTATIONDETAILSU3E_T3057255368_H
#ifndef OBJECT_T631007953_H
#define OBJECT_T631007953_H
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#pragma clang diagnostic ignored "-Wunused-variable"
#endif

// UnityEngine.Object
struct  Object_t631007953  : public RuntimeObject
{
public:
	// System.IntPtr UnityEngine.Object::m_CachedPtr
	intptr_t ___m_CachedPtr_0;

public:
	inline static int32_t get_offset_of_m_CachedPtr_0() { return static_cast<int32_t>(offsetof(Object_t631007953, ___m_CachedPtr_0)); }
	inline intptr_t get_m_CachedPtr_0() const { return ___m_CachedPtr_0; }
	inline intptr_t* get_address_of_m_CachedPtr_0() { return &___m_CachedPtr_0; }
	inline void set_m_CachedPtr_0(intptr_t value)
	{
		___m_CachedPtr_0 = value;
	}
};

struct Object_t631007953_StaticFields
{
public:
	// System.Int32 UnityEngine.Object::OffsetOfInstanceIDInCPlusPlusObject
	int32_t ___OffsetOfInstanceIDInCPlusPlusObject_1;

public:
	inline static int32_t get_offset_of_OffsetOfInstanceIDInCPlusPlusObject_1() { return static_cast<int32_t>(offsetof(Object_t631007953_StaticFields, ___OffsetOfInstanceIDInCPlusPlusObject_1)); }
	inline int32_t get_OffsetOfInstanceIDInCPlusPlusObject_1() const { return ___OffsetOfInstanceIDInCPlusPlusObject_1; }
	inline int32_t* get_address_of_OffsetOfInstanceIDInCPlusPlusObject_1() { return &___OffsetOfInstanceIDInCPlusPlusObject_1; }
	inline void set_OffsetOfInstanceIDInCPlusPlusObject_1(int32_t value)
	{
		___OffsetOfInstanceIDInCPlusPlusObject_1 = value;
	}
};

#ifdef __clang__
#pragma clang diagnostic pop
#endif
// Native definition for P/Invoke marshalling of UnityEngine.Object
struct Object_t631007953_marshaled_pinvoke
{
	intptr_t ___m_CachedPtr_0;
};
// Native definition for COM marshalling of UnityEngine.Object
struct Object_t631007953_marshaled_com
{
	intptr_t ___m_CachedPtr_0;
};
#endif // OBJECT_T631007953_H
#ifndef COMPONENT_T1923634451_H
#define COMPONENT_T1923634451_H
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#pragma clang diagnostic ignored "-Wunused-variable"
#endif

// UnityEngine.Component
struct  Component_t1923634451  : public Object_t631007953
{
public:

public:
};

#ifdef __clang__
#pragma clang diagnostic pop
#endif
#endif // COMPONENT_T1923634451_H
#ifndef BEHAVIOUR_T1437897464_H
#define BEHAVIOUR_T1437897464_H
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#pragma clang diagnostic ignored "-Wunused-variable"
#endif

// UnityEngine.Behaviour
struct  Behaviour_t1437897464  : public Component_t1923634451
{
public:

public:
};

#ifdef __clang__
#pragma clang diagnostic pop
#endif
#endif // BEHAVIOUR_T1437897464_H
#ifndef MONOBEHAVIOUR_T3962482529_H
#define MONOBEHAVIOUR_T3962482529_H
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#pragma clang diagnostic ignored "-Wunused-variable"
#endif

// UnityEngine.MonoBehaviour
struct  MonoBehaviour_t3962482529  : public Behaviour_t1437897464
{
public:

public:
};

#ifdef __clang__
#pragma clang diagnostic pop
#endif
#endif // MONOBEHAVIOUR_T3962482529_H
#ifndef BUILDINGPLACEMENT_T1065894255_H
#define BUILDINGPLACEMENT_T1065894255_H
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#pragma clang diagnostic ignored "-Wunused-variable"
#endif

// BuildingPlacement
struct  BuildingPlacement_t1065894255  : public MonoBehaviour_t3962482529
{
public:
	// UnityEngine.Transform BuildingPlacement::m_HitTransform
	Transform_t3600365921 * ___m_HitTransform_4;
	// System.Single BuildingPlacement::m_elevation
	float ___m_elevation_5;
	// UnityEngine.Vector3 BuildingPlacement::m_worldOffset
	Vector3_t3722313464  ___m_worldOffset_6;

public:
	inline static int32_t get_offset_of_m_HitTransform_4() { return static_cast<int32_t>(offsetof(BuildingPlacement_t1065894255, ___m_HitTransform_4)); }
	inline Transform_t3600365921 * get_m_HitTransform_4() const { return ___m_HitTransform_4; }
	inline Transform_t3600365921 ** get_address_of_m_HitTransform_4() { return &___m_HitTransform_4; }
	inline void set_m_HitTransform_4(Transform_t3600365921 * value)
	{
		___m_HitTransform_4 = value;
		Il2CppCodeGenWriteBarrier((&___m_HitTransform_4), value);
	}

	inline static int32_t get_offset_of_m_elevation_5() { return static_cast<int32_t>(offsetof(BuildingPlacement_t1065894255, ___m_elevation_5)); }
	inline float get_m_elevation_5() const { return ___m_elevation_5; }
	inline float* get_address_of_m_elevation_5() { return &___m_elevation_5; }
	inline void set_m_elevation_5(float value)
	{
		___m_elevation_5 = value;
	}

	inline static int32_t get_offset_of_m_worldOffset_6() { return static_cast<int32_t>(offsetof(BuildingPlacement_t1065894255, ___m_worldOffset_6)); }
	inline Vector3_t3722313464  get_m_worldOffset_6() const { return ___m_worldOffset_6; }
	inline Vector3_t3722313464 * get_address_of_m_worldOffset_6() { return &___m_worldOffset_6; }
	inline void set_m_worldOffset_6(Vector3_t3722313464  value)
	{
		___m_worldOffset_6 = value;
	}
};

#ifdef __clang__
#pragma clang diagnostic pop
#endif
#endif // BUILDINGPLACEMENT_T1065894255_H
#ifndef SHAWTILE_T2395044858_H
#define SHAWTILE_T2395044858_H
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#pragma clang diagnostic ignored "-Wunused-variable"
#endif

// Cambrian.Shaw.ShawTile
struct  ShawTile_t2395044858  : public MonoBehaviour_t3962482529
{
public:
	// UnityEngine.Vector3 Cambrian.Shaw.ShawTile::velocity
	Vector3_t3722313464  ___velocity_4;
	// UnityEngine.Vector3 Cambrian.Shaw.ShawTile::targetpt
	Vector3_t3722313464  ___targetpt_5;
	// UnityEngine.Quaternion Cambrian.Shaw.ShawTile::targetrt
	Quaternion_t2301928331  ___targetrt_6;
	// UnityEngine.Quaternion Cambrian.Shaw.ShawTile::ort
	Quaternion_t2301928331  ___ort_7;
	// System.Boolean Cambrian.Shaw.ShawTile::ashlar
	bool ___ashlar_8;
	// System.Boolean Cambrian.Shaw.ShawTile::brick
	bool ___brick_9;
	// System.Boolean Cambrian.Shaw.ShawTile::herringbone
	bool ___herringbone_10;
	// System.Boolean Cambrian.Shaw.ShawTile::stagger
	bool ___stagger_11;
	// System.Boolean Cambrian.Shaw.ShawTile::monolithic
	bool ___monolithic_12;
	// UnityEngine.GameObject Cambrian.Shaw.ShawTile::oddColumn
	GameObject_t1113636619 * ___oddColumn_13;
	// UnityEngine.GameObject Cambrian.Shaw.ShawTile::evenColumn
	GameObject_t1113636619 * ___evenColumn_14;
	// UnityEngine.GameObject Cambrian.Shaw.ShawTile::floor
	GameObject_t1113636619 * ___floor_15;
	// UnityEngine.Vector3 Cambrian.Shaw.ShawTile::oddcolumnpos
	Vector3_t3722313464  ___oddcolumnpos_16;
	// UnityEngine.Vector3[] Cambrian.Shaw.ShawTile::oddcolumntilepos
	Vector3U5BU5D_t1718750761* ___oddcolumntilepos_17;
	// UnityEngine.Vector3 Cambrian.Shaw.ShawTile::slide1
	Vector3_t3722313464  ___slide1_18;
	// UnityEngine.Vector3 Cambrian.Shaw.ShawTile::currentPosition
	Vector3_t3722313464  ___currentPosition_19;
	// System.String Cambrian.Shaw.ShawTile::currentMethod
	String_t* ___currentMethod_20;
	// Shaw.Data.ProductColor Cambrian.Shaw.ShawTile::m_productColor
	ProductColor_t905712915 * ___m_productColor_21;
	// System.Int32 Cambrian.Shaw.ShawTile::m_variationIndex
	int32_t ___m_variationIndex_22;
	// System.Collections.Generic.List`1<UnityEngine.Texture2D> Cambrian.Shaw.ShawTile::m_diffuseTextures
	List_1_t1017553631 * ___m_diffuseTextures_23;
	// System.Collections.Generic.List`1<UnityEngine.Texture2D> Cambrian.Shaw.ShawTile::m_normalTextures
	List_1_t1017553631 * ___m_normalTextures_24;
	// System.Collections.Generic.List`1<UnityEngine.Texture2D> Cambrian.Shaw.ShawTile::m_roughnessTextures
	List_1_t1017553631 * ___m_roughnessTextures_25;
	// System.Int32 Cambrian.Shaw.ShawTile::m_whichTextureType
	int32_t ___m_whichTextureType_26;

public:
	inline static int32_t get_offset_of_velocity_4() { return static_cast<int32_t>(offsetof(ShawTile_t2395044858, ___velocity_4)); }
	inline Vector3_t3722313464  get_velocity_4() const { return ___velocity_4; }
	inline Vector3_t3722313464 * get_address_of_velocity_4() { return &___velocity_4; }
	inline void set_velocity_4(Vector3_t3722313464  value)
	{
		___velocity_4 = value;
	}

	inline static int32_t get_offset_of_targetpt_5() { return static_cast<int32_t>(offsetof(ShawTile_t2395044858, ___targetpt_5)); }
	inline Vector3_t3722313464  get_targetpt_5() const { return ___targetpt_5; }
	inline Vector3_t3722313464 * get_address_of_targetpt_5() { return &___targetpt_5; }
	inline void set_targetpt_5(Vector3_t3722313464  value)
	{
		___targetpt_5 = value;
	}

	inline static int32_t get_offset_of_targetrt_6() { return static_cast<int32_t>(offsetof(ShawTile_t2395044858, ___targetrt_6)); }
	inline Quaternion_t2301928331  get_targetrt_6() const { return ___targetrt_6; }
	inline Quaternion_t2301928331 * get_address_of_targetrt_6() { return &___targetrt_6; }
	inline void set_targetrt_6(Quaternion_t2301928331  value)
	{
		___targetrt_6 = value;
	}

	inline static int32_t get_offset_of_ort_7() { return static_cast<int32_t>(offsetof(ShawTile_t2395044858, ___ort_7)); }
	inline Quaternion_t2301928331  get_ort_7() const { return ___ort_7; }
	inline Quaternion_t2301928331 * get_address_of_ort_7() { return &___ort_7; }
	inline void set_ort_7(Quaternion_t2301928331  value)
	{
		___ort_7 = value;
	}

	inline static int32_t get_offset_of_ashlar_8() { return static_cast<int32_t>(offsetof(ShawTile_t2395044858, ___ashlar_8)); }
	inline bool get_ashlar_8() const { return ___ashlar_8; }
	inline bool* get_address_of_ashlar_8() { return &___ashlar_8; }
	inline void set_ashlar_8(bool value)
	{
		___ashlar_8 = value;
	}

	inline static int32_t get_offset_of_brick_9() { return static_cast<int32_t>(offsetof(ShawTile_t2395044858, ___brick_9)); }
	inline bool get_brick_9() const { return ___brick_9; }
	inline bool* get_address_of_brick_9() { return &___brick_9; }
	inline void set_brick_9(bool value)
	{
		___brick_9 = value;
	}

	inline static int32_t get_offset_of_herringbone_10() { return static_cast<int32_t>(offsetof(ShawTile_t2395044858, ___herringbone_10)); }
	inline bool get_herringbone_10() const { return ___herringbone_10; }
	inline bool* get_address_of_herringbone_10() { return &___herringbone_10; }
	inline void set_herringbone_10(bool value)
	{
		___herringbone_10 = value;
	}

	inline static int32_t get_offset_of_stagger_11() { return static_cast<int32_t>(offsetof(ShawTile_t2395044858, ___stagger_11)); }
	inline bool get_stagger_11() const { return ___stagger_11; }
	inline bool* get_address_of_stagger_11() { return &___stagger_11; }
	inline void set_stagger_11(bool value)
	{
		___stagger_11 = value;
	}

	inline static int32_t get_offset_of_monolithic_12() { return static_cast<int32_t>(offsetof(ShawTile_t2395044858, ___monolithic_12)); }
	inline bool get_monolithic_12() const { return ___monolithic_12; }
	inline bool* get_address_of_monolithic_12() { return &___monolithic_12; }
	inline void set_monolithic_12(bool value)
	{
		___monolithic_12 = value;
	}

	inline static int32_t get_offset_of_oddColumn_13() { return static_cast<int32_t>(offsetof(ShawTile_t2395044858, ___oddColumn_13)); }
	inline GameObject_t1113636619 * get_oddColumn_13() const { return ___oddColumn_13; }
	inline GameObject_t1113636619 ** get_address_of_oddColumn_13() { return &___oddColumn_13; }
	inline void set_oddColumn_13(GameObject_t1113636619 * value)
	{
		___oddColumn_13 = value;
		Il2CppCodeGenWriteBarrier((&___oddColumn_13), value);
	}

	inline static int32_t get_offset_of_evenColumn_14() { return static_cast<int32_t>(offsetof(ShawTile_t2395044858, ___evenColumn_14)); }
	inline GameObject_t1113636619 * get_evenColumn_14() const { return ___evenColumn_14; }
	inline GameObject_t1113636619 ** get_address_of_evenColumn_14() { return &___evenColumn_14; }
	inline void set_evenColumn_14(GameObject_t1113636619 * value)
	{
		___evenColumn_14 = value;
		Il2CppCodeGenWriteBarrier((&___evenColumn_14), value);
	}

	inline static int32_t get_offset_of_floor_15() { return static_cast<int32_t>(offsetof(ShawTile_t2395044858, ___floor_15)); }
	inline GameObject_t1113636619 * get_floor_15() const { return ___floor_15; }
	inline GameObject_t1113636619 ** get_address_of_floor_15() { return &___floor_15; }
	inline void set_floor_15(GameObject_t1113636619 * value)
	{
		___floor_15 = value;
		Il2CppCodeGenWriteBarrier((&___floor_15), value);
	}

	inline static int32_t get_offset_of_oddcolumnpos_16() { return static_cast<int32_t>(offsetof(ShawTile_t2395044858, ___oddcolumnpos_16)); }
	inline Vector3_t3722313464  get_oddcolumnpos_16() const { return ___oddcolumnpos_16; }
	inline Vector3_t3722313464 * get_address_of_oddcolumnpos_16() { return &___oddcolumnpos_16; }
	inline void set_oddcolumnpos_16(Vector3_t3722313464  value)
	{
		___oddcolumnpos_16 = value;
	}

	inline static int32_t get_offset_of_oddcolumntilepos_17() { return static_cast<int32_t>(offsetof(ShawTile_t2395044858, ___oddcolumntilepos_17)); }
	inline Vector3U5BU5D_t1718750761* get_oddcolumntilepos_17() const { return ___oddcolumntilepos_17; }
	inline Vector3U5BU5D_t1718750761** get_address_of_oddcolumntilepos_17() { return &___oddcolumntilepos_17; }
	inline void set_oddcolumntilepos_17(Vector3U5BU5D_t1718750761* value)
	{
		___oddcolumntilepos_17 = value;
		Il2CppCodeGenWriteBarrier((&___oddcolumntilepos_17), value);
	}

	inline static int32_t get_offset_of_slide1_18() { return static_cast<int32_t>(offsetof(ShawTile_t2395044858, ___slide1_18)); }
	inline Vector3_t3722313464  get_slide1_18() const { return ___slide1_18; }
	inline Vector3_t3722313464 * get_address_of_slide1_18() { return &___slide1_18; }
	inline void set_slide1_18(Vector3_t3722313464  value)
	{
		___slide1_18 = value;
	}

	inline static int32_t get_offset_of_currentPosition_19() { return static_cast<int32_t>(offsetof(ShawTile_t2395044858, ___currentPosition_19)); }
	inline Vector3_t3722313464  get_currentPosition_19() const { return ___currentPosition_19; }
	inline Vector3_t3722313464 * get_address_of_currentPosition_19() { return &___currentPosition_19; }
	inline void set_currentPosition_19(Vector3_t3722313464  value)
	{
		___currentPosition_19 = value;
	}

	inline static int32_t get_offset_of_currentMethod_20() { return static_cast<int32_t>(offsetof(ShawTile_t2395044858, ___currentMethod_20)); }
	inline String_t* get_currentMethod_20() const { return ___currentMethod_20; }
	inline String_t** get_address_of_currentMethod_20() { return &___currentMethod_20; }
	inline void set_currentMethod_20(String_t* value)
	{
		___currentMethod_20 = value;
		Il2CppCodeGenWriteBarrier((&___currentMethod_20), value);
	}

	inline static int32_t get_offset_of_m_productColor_21() { return static_cast<int32_t>(offsetof(ShawTile_t2395044858, ___m_productColor_21)); }
	inline ProductColor_t905712915 * get_m_productColor_21() const { return ___m_productColor_21; }
	inline ProductColor_t905712915 ** get_address_of_m_productColor_21() { return &___m_productColor_21; }
	inline void set_m_productColor_21(ProductColor_t905712915 * value)
	{
		___m_productColor_21 = value;
		Il2CppCodeGenWriteBarrier((&___m_productColor_21), value);
	}

	inline static int32_t get_offset_of_m_variationIndex_22() { return static_cast<int32_t>(offsetof(ShawTile_t2395044858, ___m_variationIndex_22)); }
	inline int32_t get_m_variationIndex_22() const { return ___m_variationIndex_22; }
	inline int32_t* get_address_of_m_variationIndex_22() { return &___m_variationIndex_22; }
	inline void set_m_variationIndex_22(int32_t value)
	{
		___m_variationIndex_22 = value;
	}

	inline static int32_t get_offset_of_m_diffuseTextures_23() { return static_cast<int32_t>(offsetof(ShawTile_t2395044858, ___m_diffuseTextures_23)); }
	inline List_1_t1017553631 * get_m_diffuseTextures_23() const { return ___m_diffuseTextures_23; }
	inline List_1_t1017553631 ** get_address_of_m_diffuseTextures_23() { return &___m_diffuseTextures_23; }
	inline void set_m_diffuseTextures_23(List_1_t1017553631 * value)
	{
		___m_diffuseTextures_23 = value;
		Il2CppCodeGenWriteBarrier((&___m_diffuseTextures_23), value);
	}

	inline static int32_t get_offset_of_m_normalTextures_24() { return static_cast<int32_t>(offsetof(ShawTile_t2395044858, ___m_normalTextures_24)); }
	inline List_1_t1017553631 * get_m_normalTextures_24() const { return ___m_normalTextures_24; }
	inline List_1_t1017553631 ** get_address_of_m_normalTextures_24() { return &___m_normalTextures_24; }
	inline void set_m_normalTextures_24(List_1_t1017553631 * value)
	{
		___m_normalTextures_24 = value;
		Il2CppCodeGenWriteBarrier((&___m_normalTextures_24), value);
	}

	inline static int32_t get_offset_of_m_roughnessTextures_25() { return static_cast<int32_t>(offsetof(ShawTile_t2395044858, ___m_roughnessTextures_25)); }
	inline List_1_t1017553631 * get_m_roughnessTextures_25() const { return ___m_roughnessTextures_25; }
	inline List_1_t1017553631 ** get_address_of_m_roughnessTextures_25() { return &___m_roughnessTextures_25; }
	inline void set_m_roughnessTextures_25(List_1_t1017553631 * value)
	{
		___m_roughnessTextures_25 = value;
		Il2CppCodeGenWriteBarrier((&___m_roughnessTextures_25), value);
	}

	inline static int32_t get_offset_of_m_whichTextureType_26() { return static_cast<int32_t>(offsetof(ShawTile_t2395044858, ___m_whichTextureType_26)); }
	inline int32_t get_m_whichTextureType_26() const { return ___m_whichTextureType_26; }
	inline int32_t* get_address_of_m_whichTextureType_26() { return &___m_whichTextureType_26; }
	inline void set_m_whichTextureType_26(int32_t value)
	{
		___m_whichTextureType_26 = value;
	}
};

#ifdef __clang__
#pragma clang diagnostic pop
#endif
#endif // SHAWTILE_T2395044858_H
#ifndef EXAMPLEWHEELCONTROLLER_T197115271_H
#define EXAMPLEWHEELCONTROLLER_T197115271_H
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#pragma clang diagnostic ignored "-Wunused-variable"
#endif

// ExampleWheelController
struct  ExampleWheelController_t197115271  : public MonoBehaviour_t3962482529
{
public:
	// System.Single ExampleWheelController::acceleration
	float ___acceleration_4;
	// UnityEngine.Renderer ExampleWheelController::motionVectorRenderer
	Renderer_t2627027031 * ___motionVectorRenderer_5;
	// UnityEngine.Rigidbody ExampleWheelController::m_Rigidbody
	Rigidbody_t3916780224 * ___m_Rigidbody_6;

public:
	inline static int32_t get_offset_of_acceleration_4() { return static_cast<int32_t>(offsetof(ExampleWheelController_t197115271, ___acceleration_4)); }
	inline float get_acceleration_4() const { return ___acceleration_4; }
	inline float* get_address_of_acceleration_4() { return &___acceleration_4; }
	inline void set_acceleration_4(float value)
	{
		___acceleration_4 = value;
	}

	inline static int32_t get_offset_of_motionVectorRenderer_5() { return static_cast<int32_t>(offsetof(ExampleWheelController_t197115271, ___motionVectorRenderer_5)); }
	inline Renderer_t2627027031 * get_motionVectorRenderer_5() const { return ___motionVectorRenderer_5; }
	inline Renderer_t2627027031 ** get_address_of_motionVectorRenderer_5() { return &___motionVectorRenderer_5; }
	inline void set_motionVectorRenderer_5(Renderer_t2627027031 * value)
	{
		___motionVectorRenderer_5 = value;
		Il2CppCodeGenWriteBarrier((&___motionVectorRenderer_5), value);
	}

	inline static int32_t get_offset_of_m_Rigidbody_6() { return static_cast<int32_t>(offsetof(ExampleWheelController_t197115271, ___m_Rigidbody_6)); }
	inline Rigidbody_t3916780224 * get_m_Rigidbody_6() const { return ___m_Rigidbody_6; }
	inline Rigidbody_t3916780224 ** get_address_of_m_Rigidbody_6() { return &___m_Rigidbody_6; }
	inline void set_m_Rigidbody_6(Rigidbody_t3916780224 * value)
	{
		___m_Rigidbody_6 = value;
		Il2CppCodeGenWriteBarrier((&___m_Rigidbody_6), value);
	}
};

#ifdef __clang__
#pragma clang diagnostic pop
#endif
#endif // EXAMPLEWHEELCONTROLLER_T197115271_H
#ifndef TILTWINDOW_T335293945_H
#define TILTWINDOW_T335293945_H
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#pragma clang diagnostic ignored "-Wunused-variable"
#endif

// TiltWindow
struct  TiltWindow_t335293945  : public MonoBehaviour_t3962482529
{
public:
	// UnityEngine.Vector2 TiltWindow::range
	Vector2_t2156229523  ___range_4;
	// UnityEngine.Transform TiltWindow::mTrans
	Transform_t3600365921 * ___mTrans_5;
	// UnityEngine.Quaternion TiltWindow::mStart
	Quaternion_t2301928331  ___mStart_6;
	// UnityEngine.Vector2 TiltWindow::mRot
	Vector2_t2156229523  ___mRot_7;

public:
	inline static int32_t get_offset_of_range_4() { return static_cast<int32_t>(offsetof(TiltWindow_t335293945, ___range_4)); }
	inline Vector2_t2156229523  get_range_4() const { return ___range_4; }
	inline Vector2_t2156229523 * get_address_of_range_4() { return &___range_4; }
	inline void set_range_4(Vector2_t2156229523  value)
	{
		___range_4 = value;
	}

	inline static int32_t get_offset_of_mTrans_5() { return static_cast<int32_t>(offsetof(TiltWindow_t335293945, ___mTrans_5)); }
	inline Transform_t3600365921 * get_mTrans_5() const { return ___mTrans_5; }
	inline Transform_t3600365921 ** get_address_of_mTrans_5() { return &___mTrans_5; }
	inline void set_mTrans_5(Transform_t3600365921 * value)
	{
		___mTrans_5 = value;
		Il2CppCodeGenWriteBarrier((&___mTrans_5), value);
	}

	inline static int32_t get_offset_of_mStart_6() { return static_cast<int32_t>(offsetof(TiltWindow_t335293945, ___mStart_6)); }
	inline Quaternion_t2301928331  get_mStart_6() const { return ___mStart_6; }
	inline Quaternion_t2301928331 * get_address_of_mStart_6() { return &___mStart_6; }
	inline void set_mStart_6(Quaternion_t2301928331  value)
	{
		___mStart_6 = value;
	}

	inline static int32_t get_offset_of_mRot_7() { return static_cast<int32_t>(offsetof(TiltWindow_t335293945, ___mRot_7)); }
	inline Vector2_t2156229523  get_mRot_7() const { return ___mRot_7; }
	inline Vector2_t2156229523 * get_address_of_mRot_7() { return &___mRot_7; }
	inline void set_mRot_7(Vector2_t2156229523  value)
	{
		___mRot_7 = value;
	}
};

#ifdef __clang__
#pragma clang diagnostic pop
#endif
#endif // TILTWINDOW_T335293945_H





#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#pragma clang diagnostic ignored "-Wunused-variable"
#endif
extern const Il2CppTypeDefinitionSizes g_typeDefinitionSize4000 = { sizeof (RenderTextureFactory_t1946967824), -1, 0, 0 };
extern const int32_t g_FieldOffsetTable4000[1] = 
{
	RenderTextureFactory_t1946967824::get_offset_of_m_TemporaryRTs_0(),
};
extern const Il2CppTypeDefinitionSizes g_typeDefinitionSize4001 = { sizeof (ExampleWheelController_t197115271), -1, 0, 0 };
extern const int32_t g_FieldOffsetTable4001[3] = 
{
	ExampleWheelController_t197115271::get_offset_of_acceleration_4(),
	ExampleWheelController_t197115271::get_offset_of_motionVectorRenderer_5(),
	ExampleWheelController_t197115271::get_offset_of_m_Rigidbody_6(),
};
extern const Il2CppTypeDefinitionSizes g_typeDefinitionSize4002 = { sizeof (Uniforms_t1233092826), -1, sizeof(Uniforms_t1233092826_StaticFields), 0 };
extern const int32_t g_FieldOffsetTable4002[1] = 
{
	Uniforms_t1233092826_StaticFields::get_offset_of__MotionAmount_0(),
};
extern const Il2CppTypeDefinitionSizes g_typeDefinitionSize4003 = { sizeof (BuildingPlacement_t1065894255), -1, 0, 0 };
extern const int32_t g_FieldOffsetTable4003[4] = 
{
	BuildingPlacement_t1065894255::get_offset_of_m_HitTransform_4(),
	BuildingPlacement_t1065894255::get_offset_of_m_elevation_5(),
	BuildingPlacement_t1065894255::get_offset_of_m_worldOffset_6(),
	0,
};
extern const Il2CppTypeDefinitionSizes g_typeDefinitionSize4004 = { sizeof (Utility_t1436046025), -1, 0, 0 };
extern const Il2CppTypeDefinitionSizes g_typeDefinitionSize4005 = { sizeof (Product_t2706958693), -1, 0, 0 };
extern const int32_t g_FieldOffsetTable4005[2] = 
{
	Product_t2706958693::get_offset_of_name_0(),
	Product_t2706958693::get_offset_of_dpcm_1(),
};
extern const Il2CppTypeDefinitionSizes g_typeDefinitionSize4006 = { sizeof (ProductColor_t905712915), -1, 0, 0 };
extern const int32_t g_FieldOffsetTable4006[2] = 
{
	ProductColor_t905712915::get_offset_of_name_0(),
	ProductColor_t905712915::get_offset_of_variations_1(),
};
extern const Il2CppTypeDefinitionSizes g_typeDefinitionSize4007 = { sizeof (ProductVariation_t1816091343), -1, 0, 0 };
extern const int32_t g_FieldOffsetTable4007[6] = 
{
	ProductVariation_t1816091343::get_offset_of_diffusePath_0(),
	ProductVariation_t1816091343::get_offset_of_normalsPath_1(),
	ProductVariation_t1816091343::get_offset_of_roughnessPath_2(),
	ProductVariation_t1816091343::get_offset_of__diffuse_3(),
	ProductVariation_t1816091343::get_offset_of__normal_4(),
	ProductVariation_t1816091343::get_offset_of__roughness_5(),
};
extern const Il2CppTypeDefinitionSizes g_typeDefinitionSize4008 = { sizeof (InstallationMethod_t610149799), -1, 0, 0 };
extern const int32_t g_FieldOffsetTable4008[1] = 
{
	InstallationMethod_t610149799::get_offset_of_name_0(),
};
extern const Il2CppTypeDefinitionSizes g_typeDefinitionSize4009 = { sizeof (SceneLocation_t1733776779), -1, 0, 0 };
extern const int32_t g_FieldOffsetTable4009[2] = 
{
	SceneLocation_t1733776779::get_offset_of_name_0(),
	SceneLocation_t1733776779::get_offset_of_worldPosition_1(),
};
extern const Il2CppTypeDefinitionSizes g_typeDefinitionSize4010 = { sizeof (UpdateMaterialCommand_t1067309048), -1, 0, 0 };
extern const int32_t g_FieldOffsetTable4010[2] = 
{
	UpdateMaterialCommand_t1067309048::get_offset_of_product_0(),
	UpdateMaterialCommand_t1067309048::get_offset_of_color_1(),
};
extern const Il2CppTypeDefinitionSizes g_typeDefinitionSize4011 = { sizeof (UpdateInstallationCommand_t3591009552), -1, 0, 0 };
extern const int32_t g_FieldOffsetTable4011[2] = 
{
	UpdateInstallationCommand_t3591009552::get_offset_of_product_0(),
	UpdateInstallationCommand_t3591009552::get_offset_of_method_1(),
};
extern const Il2CppTypeDefinitionSizes g_typeDefinitionSize4012 = { sizeof (UpdateLocationCommand_t2892508292), -1, 0, 0 };
extern const int32_t g_FieldOffsetTable4012[1] = 
{
	UpdateLocationCommand_t2892508292::get_offset_of_location_0(),
};
extern const Il2CppTypeDefinitionSizes g_typeDefinitionSize4013 = { sizeof (ShawTile_t2395044858), -1, 0, 0 };
extern const int32_t g_FieldOffsetTable4013[23] = 
{
	ShawTile_t2395044858::get_offset_of_velocity_4(),
	ShawTile_t2395044858::get_offset_of_targetpt_5(),
	ShawTile_t2395044858::get_offset_of_targetrt_6(),
	ShawTile_t2395044858::get_offset_of_ort_7(),
	ShawTile_t2395044858::get_offset_of_ashlar_8(),
	ShawTile_t2395044858::get_offset_of_brick_9(),
	ShawTile_t2395044858::get_offset_of_herringbone_10(),
	ShawTile_t2395044858::get_offset_of_stagger_11(),
	ShawTile_t2395044858::get_offset_of_monolithic_12(),
	ShawTile_t2395044858::get_offset_of_oddColumn_13(),
	ShawTile_t2395044858::get_offset_of_evenColumn_14(),
	ShawTile_t2395044858::get_offset_of_floor_15(),
	ShawTile_t2395044858::get_offset_of_oddcolumnpos_16(),
	ShawTile_t2395044858::get_offset_of_oddcolumntilepos_17(),
	ShawTile_t2395044858::get_offset_of_slide1_18(),
	ShawTile_t2395044858::get_offset_of_currentPosition_19(),
	ShawTile_t2395044858::get_offset_of_currentMethod_20(),
	ShawTile_t2395044858::get_offset_of_m_productColor_21(),
	ShawTile_t2395044858::get_offset_of_m_variationIndex_22(),
	ShawTile_t2395044858::get_offset_of_m_diffuseTextures_23(),
	ShawTile_t2395044858::get_offset_of_m_normalTextures_24(),
	ShawTile_t2395044858::get_offset_of_m_roughnessTextures_25(),
	ShawTile_t2395044858::get_offset_of_m_whichTextureType_26(),
};
extern const Il2CppTypeDefinitionSizes g_typeDefinitionSize4014 = { sizeof (TiltWindow_t335293945), -1, 0, 0 };
extern const int32_t g_FieldOffsetTable4014[4] = 
{
	TiltWindow_t335293945::get_offset_of_range_4(),
	TiltWindow_t335293945::get_offset_of_mTrans_5(),
	TiltWindow_t335293945::get_offset_of_mStart_6(),
	TiltWindow_t335293945::get_offset_of_mRot_7(),
};
extern const Il2CppTypeDefinitionSizes g_typeDefinitionSize4015 = { sizeof (U3CPrivateImplementationDetailsU3E_t3057255368), -1, sizeof(U3CPrivateImplementationDetailsU3E_t3057255368_StaticFields), 0 };
extern const int32_t g_FieldOffsetTable4015[2] = 
{
	U3CPrivateImplementationDetailsU3E_t3057255368_StaticFields::get_offset_of_U24fieldU2D51A7A390CD6DE245186881400B18C9D822EFE240_0(),
	U3CPrivateImplementationDetailsU3E_t3057255368_StaticFields::get_offset_of_U24fieldU2DC90F38A020811481753795774EB5AF353F414C59_1(),
};
extern const Il2CppTypeDefinitionSizes g_typeDefinitionSize4016 = { sizeof (U24ArrayTypeU3D12_t2488454197)+ sizeof (RuntimeObject), sizeof(U24ArrayTypeU3D12_t2488454197 ), 0, 0 };
extern const Il2CppTypeDefinitionSizes g_typeDefinitionSize4017 = { sizeof (U24ArrayTypeU3D24_t2467506693)+ sizeof (RuntimeObject), sizeof(U24ArrayTypeU3D24_t2467506693 ), 0, 0 };
#ifdef __clang__
#pragma clang diagnostic pop
#endif
