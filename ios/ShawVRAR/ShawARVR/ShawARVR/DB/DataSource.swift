//
//  DataSource.swift
//  ShawARVR
//
//  Created by Joel Teply on 12/12/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//

import UIKit

class DataSource {
    
    let webSource = "https://shawfloors.com/api"
    
    let test = "https://shawfloors.com/api/odata/Hardwoods?$top=1000&$skip=0&$orderby=StyleSequence,UniqueId&$count=true&$select=UniqueId,SellingStyleNbr,SellingColorNbr,SellingStyleName,SellingColorName,StaticRoomFlag,Vignette,ColorCount,MSRPRange,HasSwatchImage,SampleCount,CollectionDesc,ColorFamilyDesc,Shade,SurfaceTextureShortDesc,InstallationType,CollectionDesc&$filter=(IsDropped%20eq%20false)%20and%20(ColorCount%20gt%200)%20and%20(ProductGroupPermanentName%20eq%20%27shawfloors%27)%20and%20(ProductGroupShowOnVizTool%20eq%20true)%20and%20(HasMainImage%20eq%20true)%20and%20(StaticRoomFlag%20eq%20true%20or%20HasRenderImage%20eq%20true)%20and%20(IsDefaultStyleColor%20eq%20true)"
    
    private init() {
        
    }
    
    private static var _client: DataSource?
    
    var shared: DataSource {
        get {
            if let datasource = DataSource._client {
                return datasource
            }
            let datasource = DataSource()
            DataSource._client = datasource
            return datasource
        }
    }
    
    
}
