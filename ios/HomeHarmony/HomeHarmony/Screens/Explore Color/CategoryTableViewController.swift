//
//  CategoryTableViewController.swift
//  HarmonyApp
//
//  Created by joseph on 4/14/17.
//  Copyright © 2017 Cambrian. All rights reserved.
//

import Foundation
import RealmSwift

class CategoryTableViewController: UITableViewController {
    
    var delegate: BrandCategorySelectionDelegate?
    private let cellReuseIdentifier: String = "LabelCell"
    private var table: UITableView?
    
    fileprivate var subCategories = List<BrandCategory>()
    
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        if self.subCategories.isEmpty {
            self.subCategories = getSubcategories()
        }
        
        self.tableView!.allowsMultipleSelection = false
        self.tableView.separatorStyle = .none
    }
    
    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
        
        self.navigationController?.stopUsingTransparentNavigationBar()
        self.navigationController?.navigationBar.barTintColor = appColor
        self.navigationController?.navigationBar.backgroundColor = appColor
    }
    
    
    var selectedCategory: BrandCategory?
    
    let categories = DataController.sharedInstance.brandContext?.objects(BrandCategory.self).filter({$0.parentCategory == nil})
    
    
    func getSubcategories() -> List<BrandCategory> {
        if let categories = categories {
            for category in categories {
                //print(category.name)
                if (selectedCategory == nil) {
                    selectedCategory = category
                }
                self.subCategories.append(category)
            }
        }
        return self.subCategories
    }
    
    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }
    
    // MARK: - Table view data source
    
    override func numberOfSections(in tableView: UITableView) -> Int {
        return self.subCategories.count
    }
    
    override func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        return 1
        
    }
    
    // Set the spacing between sections
    override func tableView(_ tableView: UITableView, heightForHeaderInSection section: Int) -> CGFloat {
        return 25.0
    }
    
    override func tableView(_ tableView: UITableView, viewForHeaderInSection section: Int) -> UIView? {
        let headerView = UIView()
        headerView.backgroundColor = UIColor.clear
        return headerView
    }
    
    
    override func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        let cell = tableView.dequeueReusableCell(withIdentifier: cellReuseIdentifier, for: indexPath)
        
        cell.backgroundColor = UIColor.white
        
        cell.textLabel?.text = self.subCategories[indexPath.section].name
        return cell
    }
    
    override func tableView(_ tableView: UITableView, heightForRowAt indexPath: IndexPath) -> CGFloat {
        return 100.0
    }
    
    
    override func tableView(_ tableView: UITableView, didSelectRowAt indexPath: IndexPath) {
        // set the currentCategory as the category selected
        let currentCategory = subCategories[indexPath.section]
        
        // Only perform segue if we are one step above the items
        if currentCategory.subCategories.count > 0 {
            if currentCategory.subCategories[0].items.count > 0 {
                //print(currentCategory.name)
                performSegue(withIdentifier: "showToColorCollection", sender: currentCategory)
                
            } else {
                let nextScene = self.storyboard?.instantiateViewController(withIdentifier: "CategoryTableViewController") as! CategoryTableViewController
                nextScene.subCategories = currentCategory.subCategories
                
                self.navigationController?.pushViewController(nextScene, animated: true)
                
            }
        }
    }
    
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        
        if segue.identifier == "showToColorCollection" {
            if let nextScene = segue.destination as? ExploreColorViewController {
                if let category = sender as? BrandCategory {
                    let subcategory = category.subCategories[0]
                    //print(subcategory.items.count)
                    nextScene.selectedCategory = subcategory
                }
            }
        }
    }
}

