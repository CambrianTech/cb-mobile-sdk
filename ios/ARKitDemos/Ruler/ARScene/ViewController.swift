//
//  ViewController.swift
//  ARScene
//
//  Created by Joel Teply on 6/5/17.
//  Copyright © 2017 Joel Teply. All rights reserved.
//

import UIKit
import SceneKit
import ARKit

extension SCNVector3 {
    static func positionFromTransform(_ transform: matrix_float4x4) -> SCNVector3 {
        return SCNVector3Make(transform.columns.3.x, transform.columns.3.y, transform.columns.3.z)
    }
}

enum CollisionCategory:UInt32
{
    case bottom = 1
    case cube = 2
}

class ViewController: UIViewController, ARSCNViewDelegate, ARSessionDelegate, SCNPhysicsContactDelegate {

    @IBOutlet var sceneView: ARSCNView!
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        // Set the view's delegate
        sceneView.delegate = self
        
        // Show statistics such as fps and timing information
        sceneView.showsStatistics = true
        
        // Create a new scene
        sceneView.scene = SCNScene()
        
        setupPhysics()
        
        //sceneView.debugOptions.insert(ARSCNDebugOptions.showWorldOrigin)
        //sceneView.debugOptions.insert(ARSCNDebugOptions.showFeaturePoints)
        
        let tapGesture = UITapGestureRecognizer(target: self, action: #selector(ViewController.handleTap(gestureRecognize:)))
        view.addGestureRecognizer(tapGesture)
    }
    
    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
        
        // Create a session configuration
        let configuration = ARWorldTrackingConfiguration()
        //configuration.worldAlignment = .gravityAndHeading
        configuration.planeDetection = .horizontal
        
        // Run the view's session
        sceneView.session.run(configuration)
        sceneView.session.delegate = self
    }
    
    override func viewWillDisappear(_ animated: Bool) {
        super.viewWillDisappear(animated)
        
        // Pause the view's session
        sceneView.session.pause()
    }
    
    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Release any cached data, images, etc that aren't in use.
    }

    // MARK: - ARSCNViewDelegate
    
/*
    // Override to create and configure nodes for anchors added to the view's session.
    func renderer(_ renderer: SCNSceneRenderer, nodeFor anchor: ARAnchor) -> SCNNode? {
        let node = SCNNode()
     
        return node
    }
*/
    
    func setupPhysics() {
        // For our physics interactions, we place a large node a couple of meters below the world
        // origin, after an explosion, if the geometry we added has fallen onto this surface which
        // is place way below all of the surfaces we would have detected via ARKit then we consider
        // this geometry to have fallen out of the world and remove it
        let bottomPlane = SCNBox(width: 1000, height: 0.5, length: 1000, chamferRadius: 0)
        let bottomMaterial = SCNMaterial()
        // Make it transparent so you can't see it
        bottomMaterial.diffuse.contents = UIColor(white: 1.0, alpha: 0.0)
        bottomPlane.materials = [bottomMaterial]
        let bottomNode = SCNNode(geometry: bottomPlane)
        // Place it way below the world origin to catch all falling cubes
        bottomNode.position = SCNVector3Make(0, -10, 0)
        bottomNode.physicsBody = SCNPhysicsBody(type: .kinematic, shape: nil)
        
        bottomNode.physicsBody?.categoryBitMask = Int(CollisionCategory.bottom.rawValue)
        bottomNode.physicsBody?.contactTestBitMask = Int(CollisionCategory.cube.rawValue)
        let scene: SCNScene? = sceneView.scene
        scene?.rootNode.addChildNode(bottomNode)
        scene?.physicsWorld.contactDelegate = self
    }
    
    @objc
    func handleTap(gestureRecognize: UITapGestureRecognizer) {
        // Create anchor using the camera's current position
        
        
        if gestureRecognize.state == .ended {
            
            _ = gestureRecognize.location(in: gestureRecognize.view)
            
            let tapPoint = gestureRecognize.location(in: sceneView)
        
            let results = sceneView.hitTest(tapPoint, types: .estimatedHorizontalPlane)
            
            if let filePath = Bundle.main.path(forResource: "table", ofType: "dae", inDirectory: "art.scnassets"), !results.isEmpty {
                // ReferenceNode path -> ReferenceNode URL
                
                let referenceURL = NSURL(fileURLWithPath: filePath)
                let hitResult = results[0];
                
                let insertionYOffset:Float = 0.5;
                
                let position = SCNVector3(
                    hitResult.worldTransform.columns.3.x,
                    hitResult.worldTransform.columns.3.y + insertionYOffset,
                    hitResult.worldTransform.columns.3.z
                )
                
                let referenceNode = SCNReferenceNode(url: referenceURL as URL)
                referenceNode?.load()
                //arrow.rotation = SCNVector4(0, 0, 1, -M_PI_2)
                referenceNode?.position = position
                sceneView.scene.rootNode.addChildNode(referenceNode!)
            }
//            // Create a transform with a translation of 0.2 meters in front of the camera
//            var translation = matrix_identity_float4x4
//            translation.columns.3.z = -0.2
//            let transform = simd_mul(currentFrame.camera.transform, translation)
//
//            // Add a new anchor to the session
//            let anchor = ARAnchor(transform: transform)
//            session.add(anchor: anchor)
        }
        
    }
    
    func physicsWorld(_ world: SCNPhysicsWorld, didBegin contact: SCNPhysicsContact) {
        // Here we detect a collision between pieces of geometry in the world, if one of the pieces
        // of geometry is the bottom plane it means the geometry has fallen out of the world. just remove it
//        let contactMask = contact.nodeA.physicsBody?.categoryBitMask | contact.nodeB.physicsBody?.categoryBitMask as? CollisionCategory
//        if contactMask == ([.bottom, .cube]) {
//            if contact.nodeA.physicsBody?.categoryBitMask == .bottom {
//                contact.nodeB.removeFromParentNode()
//            }
//            else {
//                contact.nodeA.removeFromParentNode()
//            }
//        }
    }

    
    func session(_ session: ARSession, didFailWithError error: Error) {
        // Present an error message to the user
        
    }
    
    func sessionWasInterrupted(_ session: ARSession) {
        // Inform the user that the session has been interrupted, for example, by presenting an overlay
        
    }
    
    func sessionInterruptionEnded(_ session: ARSession) {
        // Reset tracking and/or remove existing anchors if consistent tracking is required
        
    }
    
    func session(_ session: ARSession,
                 didUpdate frame: ARFrame) {
//        print("frame=%d", frame.camera.transform.columns.0.x,
//              frame.camera.transform.columns.0.y,
//              frame.camera.transform.columns.0.z)
    }
}
