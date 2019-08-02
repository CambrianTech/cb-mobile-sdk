//
//  AWSUtility.swift
//  HarmonyApp
//
//  Created by Jaremy Longley on 10/15/17.
//  Copyright © 2017 Cambrian. All rights reserved.
//
import Foundation
import AWSS3
import Kingfisher
import Alamofire

class AWSUtility {
    
    //go into iam->roles edit oneClick_Cognito_cambriandataUnauth_Role_1508540906601, add any of these:
    //http://docs.aws.amazon.com/AmazonS3/latest/dev/using-with-s3-actions.html
    
    //AWS connection settings
    let s3BucketName = "cambrian-data"
    let s3PoolID = "us-east-1:5807d22d-cc86-4dc9-a818-2fe9b4c4d23b"
    let baseURL: URL = URL(string: "https://s3.amazonaws.com/cambrian-data")!
    let credentialsProvider:AWSCognitoCredentialsProvider
    let configuration:AWSServiceConfiguration

    var downloadList = [String]()
    
    //timeout is just for meta information
    init (timeoutSeconds:TimeInterval=5, maxRetries:Int=2) {
        self.credentialsProvider = AWSCognitoCredentialsProvider(regionType:.USEast1,
                                                                 identityPoolId:self.s3PoolID)
        self.configuration = AWSServiceConfiguration(region: .USEast1, credentialsProvider: self.credentialsProvider)
        
        self.configuration.timeoutIntervalForRequest = timeoutSeconds
        self.configuration.maxRetryCount = UInt32(maxRetries)
        
        AWSServiceManager.default().defaultServiceConfiguration = configuration
    }
        
    func getObjectTags(_ key:String, completed: @escaping (Bool, [AWSS3Tag]) -> Void) {
        AWSS3.register(with: self.configuration, forKey: key)
        let s3 = AWSS3.s3(forKey: key)
        
        guard let listTagsRequest = AWSS3GetObjectTaggingRequest() else {
            completed(false, [AWSS3Tag]())
            return
        }
        listTagsRequest.bucket = s3BucketName
        listTagsRequest.key = key
        
        s3.getObjectTagging(listTagsRequest) { (output, error) in
            //completed(true, output.tagSet??[AWSS3Tag]())
            completed(true, output?.tagSet ?? [AWSS3Tag]())
        }
        
    }
    
    func downloadObject(_ key:String, savePath:URL,
                        progressCallback:((_ progress: Progress) -> Void)? = nil,
                        completed: @escaping (Bool) -> Void) {
        
        let remotePath = self.baseURL.appendingPathComponent(key)
        
        //print("REMOTE PATH: \(remotePath)")
        
        initiateFileDownload(remotePath,
                             savePath: savePath,
                             overwriteFiles: true,
                             progressCallback:progressCallback,
                             completionCallback:completed
        )
    }
}
