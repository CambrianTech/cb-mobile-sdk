//
//  SwatchSelectorController.swift
//  DesignUp
//
//  Created by Joel Teply on 1/23/20.
//  Copyright © 2020 Joel Teply. All rights reserved.
//

import UIKit

class SwatchSelectorController: UIViewController, UICollectionViewDelegate, UICollectionViewDataSource, UICollectionViewDelegateFlowLayout {
    
    override func viewDidLoad() {
        super.viewDidLoad()
    }
    
    func collectionView(_ collectionView: UICollectionView, numberOfItemsInSection section: Int) -> Int {
        return 0
    }
    
    func collectionView(_ collectionView: UICollectionView, cellForItemAt indexPath: IndexPath) -> UICollectionViewCell {
        let cell = collectionView.dequeueReusableCell(withReuseIdentifier: "SwatchCell", for: indexPath)
        
        return cell
    }
}
