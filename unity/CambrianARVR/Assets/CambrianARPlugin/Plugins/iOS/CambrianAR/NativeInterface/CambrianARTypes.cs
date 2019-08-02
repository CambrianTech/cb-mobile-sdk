using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System;
using System.Runtime.InteropServices;
using AOT;

namespace Cambrian.iOS {
	
	public struct CBMatrix4x4
	{
		public Vector4 column0;
		public Vector4 column1;
		public Vector4 column2;
		public Vector4 column3;

		public CBMatrix4x4(Vector4 c0, Vector4 c1, Vector4 c2, Vector4 c3)
		{
			column0 = c0; column1 = c1; column2 = c2; column3 = c3;
		}
	};
	
	public struct CBPlane
	{
		public Vector3 point;
		public Vector3 normal;
	};
	
	public struct CBTextureHandles
	{
		// Native (Metal) texture handles for the device camera buffer
		public IntPtr textureY;
		public IntPtr textureCbCr;
		public IntPtr textureMask;
	}
	
	public struct CBVideoParams
	{
		public int yWidth;
		public int yHeight;
		public int yStride;
		public uint screenOrientation;
		public float texCoordScale;
		IntPtr pYPixelBytes;
		IntPtr pUVPixelBytes;
	};

	public struct CBLightEstimation
	{
		public float ambientIntensity;
		public float ambientColorTemperature;
	};

	public struct CBDirectionalLightEstimate
	{
		public Vector4 primaryLightDirectionAndIntensity;
		public IntPtr sphericalHarmonicsCoefficients;
	};

	public enum CBLightDataType
	{
		LightEstimate,
		DirectionalLightEstimate
	};

	public struct CBLightData
	{
		public CBLightDataType lightingType;
		public CBLightEstimation lightEstimate;
		public CBDirectionalLightEstimate directionalLightEstimate;
	};
	
	public enum CBTrackingState
	{
		UnityARTrackingStateNotAvailable,
		UnityARTrackingStateLimited,
		UnityARTrackingStateNormal,
	};

	public enum CBTrackingReason
	{
		UnityARTrackingStateReasonNone,
		UnityARTrackingStateReasonInitializing,
		UnityARTrackingStateReasonExcessiveMotion,
		UnityARTrackingStateReasonInsufficientFeatures,
	};

	public struct CBCameraFrame
	{
		public CBMatrix4x4 cameraTransform;
		public CBMatrix4x4 projectionMatrix;
		public CBTrackingState trackingState;
		public CBTrackingReason trackingReason;
		public CBVideoParams videoParams;
		public CBLightData lightData;
		public CBMatrix4x4 displayTransform;
	};

	public struct CBPoint2D {
		public float x;
		public float y;
	}
	
	public enum CBHitTestResultType : long
	{
		/** Result type from intersecting the nearest feature point. */
		FeaturePoint     = (1 << 0),

		/** Result type from detecting and intersecting a new horizontal plane. */
		EstimatedHorizontalPlane  = (1 << 1),

		/** Result type from detecting and intersecting a new vertical plane. */
		EstimatedVerticalPlane    = (1 << 2),

		/** Result type from intersecting with an existing plane anchor. */
		ExistingPlane    = (1 << 3),

		/** Result type from intersecting with an existing plane anchor, taking into account the plane's extent. */
		ExistingPlaneUsingExtent  = ( 1 << 4),
		
		/** A point on a real-world plane (already detected with the planeDetection option), respecting the plane's estimated size and shape */
		ExistingPlaneUsingGeometry = ( 1 << 5)
	}
	
	public struct CBHitTestResult
	{
		/**
		 The type of the hit-test result.
		 */
		public CBHitTestResultType type;

		/**
 		The distance from the camera to the intersection in meters.
		*/
		public double distance;

		/**
 		The transformation matrix that defines the intersection's rotation, translation and scale
 		relative to the anchor or nearest feature point.
 		*/
		public CBMatrix4x4 localTransform;

		/**
 		The transformation matrix that defines the intersection's rotation, translation and scale
 		relative to the world.
 		*/
		public CBMatrix4x4 worldTransform;
		
	}
	
	public enum CBPlaneAnchorAlignment : long
	{
		/** A plane that is horizontal with respect to gravity. */
		Horizontal,

		/** A plane that is parallel with respect to gravity. */
		Vertical
	}
	
	public struct CBPlaneGeometryMarshaled
	{
		public int vertexCount;
		public IntPtr vertices;
		public int textureCoordinateCount;
		public IntPtr textureCoordinates;
		public int triangleCount;
		public IntPtr triangleIndices;
		public int boundaryVertexCount;
		public IntPtr boundaryVertices;

	}
	
	public class CBPlaneGeometry
	{
		private CBPlaneGeometryMarshaled uPlaneGeometry;

		public CBPlaneGeometry (CBPlaneGeometryMarshaled pgm)
		{
			uPlaneGeometry = pgm;
		}

		public int vertexCount { get { return uPlaneGeometry.vertexCount; } }
		public int triangleCount {  get  { return uPlaneGeometry.triangleCount; } }
		public int textureCoordinateCount { get { return uPlaneGeometry.textureCoordinateCount; } }
		public int boundaryVertexCount { get { return uPlaneGeometry.boundaryVertexCount; } }

		public Vector3 [] vertices { get { return MarshalVertices(uPlaneGeometry.vertices,vertexCount); } }

		public Vector3 [] boundaryVertices { get { return MarshalVertices(uPlaneGeometry.boundaryVertices,boundaryVertexCount); } }

		public Vector2 [] textureCoordinates { get { return MarshalTexCoords(uPlaneGeometry.textureCoordinates, textureCoordinateCount); } }

		public int [] triangleIndices { get { return MarshalIndices(uPlaneGeometry.triangleIndices, triangleCount); } }

		Vector3 [] MarshalVertices(IntPtr ptrFloatArray, int vertCount)
		{
			int numFloats = vertCount * 4;
			float [] workVerts = new float[numFloats];
			Marshal.Copy (ptrFloatArray, workVerts, 0, (int)(numFloats)); 

			Vector3[] verts = new Vector3[vertCount];

			for (int count = 0; count < numFloats; count++)
			{
				verts [count / 4].x = workVerts[count++];
				verts [count / 4].y = workVerts[count++];
				verts [count / 4].z = -workVerts[count++];
			}

			return verts;
		}

		int [] MarshalIndices(IntPtr ptrIndices, int triCount)
		{
			int numIndices = triCount * 3;
			short [] workIndices = new short[numIndices];  //since ARKit returns Int16
			Marshal.Copy (ptrIndices, workIndices, 0, numIndices);

			int[] triIndices = new int[numIndices];
			for (int count = 0; count < numIndices; count+=3) {
				//reverse winding order
				triIndices [count] = workIndices [count];
				triIndices [count + 1] = workIndices [count + 2];
				triIndices [count + 2] = workIndices [count + 1];
			}

			return triIndices;
		}

		Vector2 [] MarshalTexCoords(IntPtr ptrTexCoords, int texCoordCount)
		{
			int numFloats = texCoordCount * 2;
			float [] workTexCoords = new float[numFloats];
			Marshal.Copy (ptrTexCoords, workTexCoords, 0, (int)(numFloats)); 

			Vector2[] texCoords = new Vector2[texCoordCount];

			for (int count = 0; count < numFloats; count++)
			{
				texCoords [count / 2].x = workTexCoords[count++];
				texCoords [count / 2].y = workTexCoords[count];
			}

			return texCoords;

		}
	}
	
	public struct CBPlaneAnchor 
	{

		public string identifier;

		/**
 		The transformation matrix that defines the anchor's rotation, translation and scale in world coordinates.
		 */
		public CBMatrix4x4 transform;

		/**
		 The alignment of the plane.
		 */

		public CBPlaneAnchorAlignment alignment;

		/**
		The center of the plane in the anchor’s coordinate space.
		*/

		public Vector3 center;

		/**
		The extent of the plane in the anchor’s coordinate space.
		 */
		public Vector3 extent;

		
		/**
		Contours of plane, triangles, etc
		 */
		public CBPlaneGeometryMarshaled _geometryRaw;

		public CBPlaneGeometry geometry {
			get {
				return new CBPlaneGeometry(_geometryRaw);
			}
		}
	}
	
	public struct CBUserAnchor 
	{

		public string identifier;

		/**
 		The transformation matrix that defines the anchor's rotation, translation and scale in world coordinates.
		 */
		public CBMatrix4x4 transform;
	}


	public enum CBAssetType {
		All = 0,
		Paint,
		Floor,
		Model,
	}
	
	public enum CBMode {
		Video = 0,
		Still,
		Capture,
	}
	
	[StructLayout(LayoutKind.Sequential)]
	public struct CambrianARSessionState {
		public bool isVideo;
		public Matrix4x4 worldTransform;
	}

	[StructLayout(LayoutKind.Sequential)]
	public struct DisplayParams {
		public Matrix4x4 displayTransform;
		public int width;
		public int height;
	}
	
	[StructLayout(LayoutKind.Sequential)]
	public struct CambrianARImage
	{
		public IntPtr image;
		public int channels;
		public int width;
		public int height;
	}
	
	[StructLayout(LayoutKind.Sequential)]
	public struct DrawingParams
	{		
		public float colorRed;
		public float colorGreen;
		public float colorBlue;
		public float colorAlpha;
	}
	
	[StructLayout(LayoutKind.Sequential)]
	public struct CambrianScreenshot
	{
		public IntPtr pathPtr;
		public string path { get { return Marshal.PtrToStringAuto(this.pathPtr); } }
		
		public bool isJPEG;
		public int qualityLevel;
	}

    [StructLayout(LayoutKind.Sequential)]
    public struct CambrianCommand
    {
        public IntPtr commandPtr;
        public string commandString { get { return Marshal.PtrToStringAuto(this.commandPtr); } }

        public IntPtr jsonPtr;
        public string jsonString { get { return Marshal.PtrToStringAuto(this.jsonPtr); } }
    }
	
	[StructLayout(LayoutKind.Sequential)]
	public struct CambrianARAsset
	{
		public int index;
		public CBAssetType assetType;
		public Matrix4x4 transform;
	}
	
	[StructLayout(LayoutKind.Sequential)]
	public struct CambrianARPaint 
	{
		public int index;
		public CBAssetType assetType;
		
		public float colorRed;
		public float colorGreen;
		public float colorBlue;
		public float colorAlpha;
	}

	[StructLayout(LayoutKind.Sequential)]
	public struct CambrianARFloor 
	{
		public int index;
		public CBAssetType assetType;
		
		public float scale;
		
		public IntPtr diffusePathPtr;
		public string diffusePath { get { return Marshal.PtrToStringAuto(this.diffusePathPtr); } }
		
		public IntPtr normalPathPtr;
		public string normalPath { get { return Marshal.PtrToStringAuto(this.normalPathPtr); } }
		
		public IntPtr roughnessPathPtr;
		public string roughnessPath { get { return Marshal.PtrToStringAuto(this.roughnessPathPtr); } }
	}
	
	[StructLayout(LayoutKind.Sequential)]
	public struct CambrianARModel
	{
		public IntPtr assetIDPtr;
		public CBAssetType assetType;
		
		public string assetID { get { return Marshal.PtrToStringAuto(this.assetIDPtr); } }
		
		public float scale;
		
		public IntPtr modelPathPtr;
		public string modelPath { get { return Marshal.PtrToStringAuto(this.modelPathPtr); } }
	}
	
	[StructLayout(LayoutKind.Sequential)]
	public struct CBSurfaceAssetParams
	{
		public int index;
		public CBAssetType assetType;

		public int maskWidth;
		public int maskHeight;
		
		public int shadowWidth;
		public int shadowHeight;
	}
	
	[StructLayout(LayoutKind.Sequential)]
	public struct CBMeshData {
		public int index;
		public Vector3[] vertices;
		public int[] indices;
	}
	
	public class CBSurfaceTextureData {
		public int index;
		
		public CBSurfaceAssetParams asset;
		
		public Texture2D maskTexture;
		public Texture2D shadowsTexture;
	}

	[StructLayout(LayoutKind.Sequential)]
	public class CBImageData
	{
		public IntPtr image;
		public int channels = 1;
		public int width;
		public int height;
	}
	
	[StructLayout(LayoutKind.Sequential)]
	public class CBSurfaceDataExtents
	{
		public Vector3 center;
		public Vector3 normal;
		public Vector2 rotation2D;
		
		public Vector4 meshExtents3D;
		public Vector4 maskExtents3D;
		public Vector4 shadowsExtents3D;
	}
	
	public class CBSurfaceData
	{
		public CBSurfaceData(CBSurfaceDataExtents dataExtents)
		{
			extents = dataExtents;
		}
		
		public CBMeshData mesh;
		public CBSurfaceDataExtents extents;
		
		public byte[] maskBytes;
		public byte[] shadowBytes;
	}
	
    [StructLayout(LayoutKind.Sequential)]
    public struct CambrianARLineCollection
    {
        public Vector3[] points;
        public Color color;
    }
}
