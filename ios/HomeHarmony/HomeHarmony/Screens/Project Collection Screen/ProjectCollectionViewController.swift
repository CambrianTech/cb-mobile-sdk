//
//  File.swift
//  HarmonyApp
//
//  Created by joseph on 5/8/17.
//  Copyright © 2017 Cambrian. All rights reserved.
//

import Foundation
import RealmSwift
import Kingfisher

internal protocol ProjectCellDelegate : NSObjectProtocol {
    func projectSelected(_ project:VisualizerProject)
    func addCollectionViewItem()
}

class ProjectCell: UICollectionViewCell {
    
    static let reuseIdentifier = "ProjectCell"
    
    @IBOutlet weak var imageView: UIImageView!
    @IBOutlet weak var projectLabel: UILabel!
    @IBOutlet weak var dateLabel: UILabel!
    
    weak fileprivate var delegate: ProjectCellDelegate?
    
    var project: VisualizerProject? {
        didSet {
            
            projectLabel.text = project?.name
            if let image = project?.currentImage {
                let resource = ImageResource(downloadURL: image.previewImagePath!,
                                             cacheKey: String(describing: image.modified))
                imageView.kf.setImage(with: resource)
            } else {
                imageView.image = UIImage(named: "ic_image_gallery")
                imageView.contentMode = .scaleAspectFit
            }
            
            if !imageView.isUserInteractionEnabled {
                let singleTap = UITapGestureRecognizer(target: self, action:#selector(self.clickedImage))
                singleTap.numberOfTapsRequired = 1
                imageView.isUserInteractionEnabled = true
                imageView.addGestureRecognizer(singleTap)
            }
            
            
            self.layer.cornerRadius = 3
            self.layer.shadowColor = UIColor.black.cgColor
            self.layer.shadowRadius = 1
            self.layer.shadowOpacity = 0.3
            self.layer.shadowOffset = CGSize(width: 0.5, height: 1.2)
            
            self.clipsToBounds = false
            
            self.setNeedsDisplay()
        }
    }
    
    func setDate() {
        let formatter = DateFormatter()
        let dateString: String
        formatter.dateFormat = "LLL yyyy"
        if let modified = project?.modified {
            dateString = formatter.string(from: modified)
        } else {
            dateString = formatter.string(from: project!.created)
        }
        
        print(dateString)
        self.dateLabel.text = dateString
    }
    
    @objc func clickedImage() {
        if let proj = self.project {
            self.delegate?.projectSelected(proj)
        }
    }
}

class AddProjectCell: UICollectionViewCell {
    weak fileprivate var delegate: ProjectCellDelegate?
}

class AllProjectsCollection: UICollectionView {
    
}

class ProjectCollectionViewController: UICollectionViewController, ProjectCellDelegate, DetailsCollectionViewDelegate, UICollectionViewDelegateFlowLayout {
    
    weak internal var delegate: ProjectCellDelegate?
    var selectedProject: VisualizerProject?
    var addButton: UIButton?
    var backButonText: String? = "Home"
    
    fileprivate var projects: Results<VisualizerProject> {
        get {
            return VisualizerProject.latestProjects()
        }
    }
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        let size:CGFloat = 60
        navigationItem.setLeftBarButtonItems(CustomBackButton.createWithText(
            text: self.backButonText!,
            color: UIColor.black,
            target: self,
            action: #selector(self.backPressed)), animated: true)
    }
    
    
    @objc func backPressed() {
        self.navigationController?.popViewController(animated: true)
    }
    
    func collectionView(_ collectionView: UICollectionView, layout collectionViewLayout: UICollectionViewLayout, minimumInteritemSpacingForSectionAt section: Int) -> CGFloat {
        return 0
    }
    
    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
        self.navigationController?.stopUsingTransparentNavigationBar()
        self.navigationController?.navigationBar.barTintColor = appColor
        self.navigationController?.navigationBar.backgroundColor = appColor
        reloadData()
    }
    
    func reloadData() {
        self.collectionView?.reloadData()
    }
    
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        if let vc = segue.destination as? ARViewController {
            vc.backButtonText = "Projects"
        } else if let vc = segue.destination as? ProjectDetailsViewController {
            if let project = self.selectedProject {
                VisualizerProject.currentProject = project
            }
            vc.backButtonText = "Projects"
        }
    }
    
    // MARK: UICollectionViewDataSource
    override func numberOfSections(in collectionView: UICollectionView) -> Int {
        // #warning Incomplete implementation, return the number of sections
        return 2
    }
    
    
    override func collectionView(_ collectionView: UICollectionView, numberOfItemsInSection section: Int) -> Int {
        // #warning Incomplete implementation, return the number of items
        if section == 0 {
            return 1
        } else {
            return self.projects.count + 1
        }
    }
    
    override func collectionView(_ collectionView: UICollectionView, cellForItemAt indexPath: IndexPath) -> UICollectionViewCell {
        if indexPath.section == 0 {
            guard let cell = collectionView.dequeueReusableCell(withReuseIdentifier: "CurrentProjectCell", for: indexPath) as? ProjectCell
                else {
                    fatalError("CurrentProjectCell expected")
            }
            
            cell.project = self.projects[(indexPath as NSIndexPath).row]
            cell.delegate = self
            cell.setDate()
            
            return cell
        }
        else {
            if indexPath.row == 0 {
                // add item cell
                guard let cell = collectionView.dequeueReusableCell(withReuseIdentifier: "AddProjectCell", for: indexPath) as? AddProjectCell else  {
                    fatalError("Expected AddProjectCell")
                }
                cell.delegate = self.delegate
                cell.layer.cornerRadius = 3
                cell.layer.shadowColor = UIColor.black.cgColor
                cell.layer.shadowRadius = 1
                cell.layer.shadowOpacity = 0.3
                cell.layer.shadowOffset = CGSize(width: 0.5, height: 1.2)
                
                cell.clipsToBounds = false
                
                cell.setNeedsDisplay()
                return cell
            } else {
                guard let cell = collectionView.dequeueReusableCell(withReuseIdentifier: "ProjectCell", for: indexPath) as? ProjectCell else  {
                    fatalError("Expected ProjectCell")
                }
                cell.project = self.projects[indexPath.row - 1]
                cell.delegate = self
                return cell
            }
        }
    }
    
    func collectionView(_ collectionView: UICollectionView, layout collectionViewLayout: UICollectionViewLayout, sizeForItemAt indexPath: IndexPath) -> CGSize {
        if indexPath.section == 0 {
            let width = (self.view.frame.width) - 20
            let height = 250
            return CGSize(width: Int(width), height: height)
        } else {
            let height = 150
            let width = (collectionView.frame.width / 2.25)
            return CGSize(width: Int(width), height: height)
        }
        
    }
    
    func projectSelected(_ project:VisualizerProject) {
        self.selectedProject = project
        if (project.isEmpty) {
            self.performSegue(withIdentifier: "showPainter", sender: self)
        }
        else {
            self.performSegue(withIdentifier: "showDetails", sender: self)
        }
    }
    
    override func collectionView(_ collectionView: UICollectionView, didSelectItemAt indexPath: IndexPath) {
        if indexPath.section == 0 {
            let project = self.projects[(indexPath as NSIndexPath).row]
            self.projectSelected(project)
        } else {
            if indexPath.row == 0 {
                self.addCollectionViewItem()
            } else {
                self.projectSelected(self.projects[(indexPath as NSIndexPath).row])
            }
        }
    }
    
    // MARK: DetailsCollectionViewDelegate
    func canAddCollectionViewItem() -> Bool {
        return true
    }
    
    func addCollectionViewItem() {
        VisualizerProject.currentProject = VisualizerProject.createProject()
        reloadData()
        
        VisualizerProject.currentProject.renameAlert(self, handler: { (success) in
            if (success) {
                self.editProject(VisualizerProject.currentProject)
            } else {
                VisualizerProject.currentProject.deleteProject()
                self.reloadData()
            }
        })
    }
    
    func longPressCollectionViewItem(_ indexPath:IndexPath) {
        let project = self.projects[(indexPath as NSIndexPath).row]
        
        let item = collectionView?.dequeueReusableCell(withReuseIdentifier: "ProjectCell", for: indexPath) as? ProjectCell
        
        let optionMenu = UIAlertController(title: nil, message: "Project Changes", preferredStyle: .actionSheet)
        
        let detailsAction = UIAlertAction(title: "Project Details", style: .default, handler: {
            (alert: UIAlertAction!) -> Void in
            self.projectSelected(project)
        })
        
        let editAction = UIAlertAction(title: "See It", style: .default, handler: {
            (alert: UIAlertAction!) -> Void in
            self.editProject(project)
        })
        
        let renameAction = UIAlertAction(title: "Rename Project", style: .default, handler: {
            (alert: UIAlertAction!) -> Void in
            
            project.renameAlert(self, handler: { (success) in
                if (success) {
                    self.collectionView?.reloadItems(at: [indexPath])
                }
            })
        })
        
        let deleteAction = UIAlertAction(title: "Delete Project", style: .destructive, handler: {
            (alert: UIAlertAction!) -> Void in
            
            confirmAction(self, text: "Are you sure you want to delete this project?", completion: {
                project.deleteProject({
                    self.reloadData()
                })
            })
        })
        
        let cancelAction = UIAlertAction(title: "Cancel", style: .cancel, handler:nil)
        
        optionMenu.addAction(editAction)
        optionMenu.addAction(detailsAction)
        optionMenu.addAction(renameAction)
        optionMenu.addAction(deleteAction)
        optionMenu.addAction(cancelAction)
        
        
        presentActionSheet(optionMenu, viewController: self, view: item!)
    }
    
    func editProject(_ project:VisualizerProject) {
        VisualizerProject.currentProject = project
        
        self.performSegue(withIdentifier: "showPainter", sender: self)
    }
}
