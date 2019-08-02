//
//  RoomCalculator.swift
//  Prestige
//
//  Created by Joel Teply on 6/6/16.
//  Copyright © 2016 Cambrian. All rights reserved.
//

import Foundation

class RoomCalculatorViewController: UIViewController, UITextFieldDelegate {
    
    @IBOutlet weak var interiorButton: UIButton!
    @IBOutlet weak var exteriorButton: UIButton!
    
    @IBOutlet weak var widthTextBox: UITextField!
    @IBOutlet weak var heightTextBox: UITextField!
    
    @IBOutlet weak var windowNumberLabel: UILabel!
    @IBOutlet weak var doorNumberLabel: UILabel!
    @IBOutlet weak var coatsControl: UISegmentedControl!
    
    @IBOutlet weak var calculatedArea: UILabel!
    @IBOutlet weak var calculatedAmount: UILabel!
    
    var isInterior = true
    var totalWidth = 0
    var wallHeight = 9
    var numWindows = 0
    var numDoors = 0
    var numCoats = 1

    
    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
        self.update()
    }
    
    @IBAction func interiorPressed(_ sender: AnyObject) {
        self.isInterior = true
        self.update()
    }
    
    @IBAction func exteriorPressed(_ sender: AnyObject) {
        self.isInterior = false
        self.update()
    }
    
    @IBAction func windowRemovePressed(_ sender: AnyObject) {
        self.numWindows = max(numWindows - 1, 0)
        
        self.update()
    }
    
    @IBAction func windowAddPressed(_ sender: AnyObject) {
        self.numWindows += 1
        
        self.update()
    }
    
    @IBAction func doorRemovePressed(_ sender: AnyObject) {
        self.numDoors = max(numDoors - 1, 0)
        
        self.update()
    }
    
    @IBAction func doorAddPressed(_ sender: AnyObject) {
        self.numDoors += 1
        
        self.update()
    }
    
    @IBAction func coatsChanged(_ sender: AnyObject) {
        self.numCoats = self.coatsControl.selectedSegmentIndex + 1
        self.update()
    }
    
    func textFieldShouldReturn(_ textField: UITextField) -> Bool {
        textField.resignFirstResponder()
        return true
    }
    
    func textFieldDidEndEditing(_ textField: UITextField) {
        if textField == self.widthTextBox {
            self.totalWidth = Int(self.widthTextBox.text!) ?? 0
        } else if textField == self.heightTextBox {
            self.wallHeight = Int(self.heightTextBox.text!) ?? 0
        }
        
        self.update()
    }
    
    func textFieldShouldBeginEditing(_ textField: UITextField) -> Bool {

        //textField.selectedTextRange = textField.textRangeFromPosition(textField.beginningOfDocument, toPosition: textField.endOfDocument)
        
        DispatchQueue.main.async(execute: {
            textField.selectedTextRange = textField.textRange(from: textField.beginningOfDocument, to: textField.endOfDocument)
        });

        // Create a button bar for the number pad
        let keyboardDoneButtonView = UIToolbar()
        keyboardDoneButtonView.sizeToFit()
        
        // Setup the buttons to be put in the system.
        let item = UIBarButtonItem(title: "Done", style: .plain, target: self, action: #selector(numberEndEditing) )
        let toolbarButtons = [item]
        
        //Put the buttons into the ToolBar and display the tool bar
        keyboardDoneButtonView.setItems(toolbarButtons, animated: false)
        textField.inputAccessoryView = keyboardDoneButtonView
        
        return true
    }
    
    override func touchesBegan(_ touches: Set<UITouch>, with event: UIEvent?) {
        self.view.endEditing(true)
    }
    
    @objc func numberEndEditing() {
         self.view.endEditing(true)
    }
    
    func update() {
        self.interiorButton.backgroundColor = self.isInterior ? UIColor.white : UIColor.lightGray
        self.exteriorButton.backgroundColor = !self.isInterior ? UIColor.white : UIColor.lightGray
        
        self.widthTextBox.text = "\(self.totalWidth)"
        self.heightTextBox.text = "\(self.wallHeight)"
        
        self.windowNumberLabel.text = "\(self.numWindows)"
        self.doorNumberLabel.text = "\(self.numDoors)"
        
        self.coatsControl.selectedSegmentIndex = self.numCoats - 1
        
        let totalArea = getTotalArea()
        
        self.calculatedArea.isHidden = totalArea == 0
        self.calculatedAmount.isHidden = self.calculatedArea.isHidden
        
        if (totalArea > 0 ){
            self.calculatedArea.text = "\(Int(totalArea)) sq. ft."
            
            let numCans = getNumCans()
            let cansText = numCans == 1 ? "can" : "cans"
            self.calculatedAmount.text = "\(numCans) \(cansText)"
        }
        
    }
    
    func getNumCans() -> Int {
        let denom = isInterior ? sqFfPerInterior : sqFfPerExterior
        let numCans:Float = Float(numCoats) * getTotalArea() / denom
        return max(1, Int(ceil(numCans)))
    }
    
    func getTotalArea() -> Float {
        let wallArea = Float(self.totalWidth * self.wallHeight)
        
        let doorArea = doorSizeSqFt * Float(numDoors)
        let windowArea = windowSizeSqFt * Float(numWindows)
        
        return max(0, (wallArea - doorArea - windowArea))
    }
}
