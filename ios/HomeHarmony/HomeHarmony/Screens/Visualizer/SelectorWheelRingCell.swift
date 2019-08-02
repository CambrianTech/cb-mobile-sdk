

import UIKit
import Foundation
import RealmSwift

//http://stackoverflow.com/questions/32771864/draw-text-along-circular-path-in-swift-for-ios

class SelectorWheelRingCell: UICollectionViewCell {
    
    fileprivate var cellText = UILabel()
    
    var angleSpan:CGFloat = 0.2
    var isLandscape = false
    var innerRadius:CGFloat = 0
    var outerRadius:CGFloat = 0
    var innerRadiusClipping:CGFloat = 10
    
    private var path = UIBezierPath()
    private var isEnlarged = false
    private var hasShadow = false
    
    fileprivate func applyClipping() {
        
        let startAngle = 1.5 * Float.pi - Float(angleSpan) / 2.0
        let endAngle = startAngle + Float(angleSpan)
        
        let center = CGPoint(x: self.bounds.size.width / 2.0, y: outerRadius)
        
        let clippedRadius = innerRadius + innerRadiusClipping
        
        let outerArcStart = CGPoint(
            x: center.x + outerRadius * CGFloat(cosf(startAngle)),
            y: center.y + outerRadius * CGFloat(sinf(startAngle)))
        
        let innerArcEnd = CGPoint(
            x: center.x + clippedRadius * CGFloat(cosf(endAngle)),
            y: center.y + clippedRadius * CGFloat(sinf(endAngle)))
        
        path = UIBezierPath()
        path.move(to: outerArcStart)
        
        //stroke outer arc
        path.addArc(withCenter: center, radius: self.outerRadius, startAngle: CGFloat(startAngle), endAngle: CGFloat(endAngle), clockwise: true)
        path.addLine(to: innerArcEnd)
        
        //stroke inner arc
        path.addArc(withCenter: center, radius: clippedRadius, startAngle: CGFloat(endAngle), endAngle: CGFloat(startAngle), clockwise: false)
        path.addLine(to: outerArcStart)
        
        path.close()
        
        //        if let view = self.contentView.subviews.first {
        //            view.frame = CGRect(x:self.bounds.minX, y:self.bounds.minY + 10,
        //                                width:self.bounds.width, height:self.bounds.height-10)
        //        }
        
        let edgeLayer = CAShapeLayer()
        
        edgeLayer.frame = self.bounds
        edgeLayer.path = path.cgPath
        edgeLayer.strokeColor = UIColor.black.withAlphaComponent(0.3).cgColor
        edgeLayer.fillColor = nil;
        edgeLayer.lineWidth = 2.0;
        contentView.layer.addSublayer(edgeLayer)
        
        let mask = CAShapeLayer()
        mask.frame = self.bounds
        mask.path = path.cgPath
        
        contentView.layer.mask = mask
    }
    
    override func point(inside point: CGPoint, with event: UIEvent?) -> Bool {
        if self.path.contains(point) {
            //print("touch is inside cell")
            return true
        }
        //print("touch is outside cell")
        return false
    }
    
    fileprivate func initialize() {
        //contentView.layer.cornerRadius = 10
        
        contentView.layer.shouldRasterize = true
        contentView.layer.rasterizationScale = UIScreen.main.scale
        contentView.clipsToBounds = false
        
        let height:CGFloat = self.frame.size.height * 0.8
        let width = self.frame.size.width * 0.7
        
        cellText.frame = CGRect(x: (self.frame.size.width - width) / 2,
                                y: (self.frame.size.height - height) / 6 - 10,
                                width: width, height: height)
        
        cellText.font = UIFont.systemFont(ofSize: 15)
        cellText.textAlignment = .center
        cellText.numberOfLines = 0
        
        let screen = UIScreen.main.bounds
        if(screen.width > screen.height) {
            //cellText.transform = CGAffineTransform(rotationAngle: CGFloat.pi / 2)
        }
        applyPlainShadow(self, offset: CGSize(), radius: 0, opacity: 0.7)
    }
    
    override init(frame: CGRect) {
        super.init(frame: frame)
        initialize()
    }
    
    required init?(coder aDecoder: NSCoder) {
        super.init(coder: aDecoder)
        initialize()
    }
    
    override func apply(_ layoutAttributes: UICollectionViewLayoutAttributes) {
        super.apply(layoutAttributes)
        self.layer.shadowColor = UIColor.clear.cgColor
        self.layer.shadowOpacity = 0.0
        
        if let circularlayoutAttributes = layoutAttributes as? SelectorWheelRingLayoutAttributes {
            self.layer.anchorPoint = circularlayoutAttributes.anchorPoint
            let height = (self.outerRadius == self.innerRadius) ? self.bounds.height : self.outerRadius - self.innerRadius
            self.center.y += (circularlayoutAttributes.anchorPoint.y - 0.5) * height
        }
        
    }
    
    
    func setEnlarged(_ enlarged:Bool, animated:Bool) {
        
        
        if enlarged {
            applyPlainShadow(self, offset: CGSize(), radius:10, opacity: enlarged ? 0.7 : 0.0)
        } else {
            self.layer.shadowColor = UIColor.clear.cgColor
        }
        
        let scale:CGFloat = enlarged ? 1.05 : 1.0
        //let duration: TimeInterval = animated ? 0.18 : 0
        
        let t = self.transform;
        
        let existingScale = sqrt(t.a * t.a + t.c * t.c);
        self.isEnlarged = existingScale > 1.00
        
        if (self.isEnlarged && enlarged) {
            return
        }
        
        self.superview?.bringSubview(toFront: self)
        
        if (animated) {
            UIView.animate(withDuration: 0.2, delay: 0.01, options: .curveEaseOut, animations: {
                self.transform = self.transform.scaledBy(x: scale, y: scale)
            }, completion: { finished in
            })
        } else {
            self.transform = self.transform.scaledBy(x: scale, y: scale)
        }
        
    }
    
    func setDrawable(_ drawable:Drawable) {
        
        for view in self.contentView.subviews {
            view.removeFromSuperview()
        }
        
        if let view = drawable.view {
            view.frame = self.frame
            view.frame.origin = CGPoint()
            self.contentView.addSubview(view)
        }
        
        self.contentView.addSubview(self.cellText)
        
        let dark = UIColor.init(red: 0.1, green: 0.1, blue: 0.1, alpha: 0.8)
        
        cellText.text = drawable.text
        cellText.textColor = drawable.needsDarkText() ? dark : UIColor.lightText
        
        applyClipping()
    }
}

class SelectorWheelRingLayoutAttributes: UICollectionViewLayoutAttributes {
    
    var anchorPoint = CGPoint(x: 0.5, y: 0.5)
    
    var angle: CGFloat = 0 {
        didSet {
            zIndex = Int(angle*1000000)
            transform = CGAffineTransform(rotationAngle: angle)
        }
    }
    
    override func copy(with zone: NSZone?) -> Any {
        let copiedAttributes: SelectorWheelRingLayoutAttributes = super.copy(with: zone) as! SelectorWheelRingLayoutAttributes
        copiedAttributes.anchorPoint = self.anchorPoint
        copiedAttributes.angle = self.angle
        return copiedAttributes
    }
    
}

class SelectorWheelRingLayout: UICollectionViewLayout {
    
    var itemSize = CGSize(width: 133, height: 100) {
        didSet {
            invalidateLayout()
        }
    }
    
    var angleAtExtreme: CGFloat {
        if collectionView!.numberOfItems(inSection: 0) > 1 {
            let angleAtExtreme = -CGFloat(collectionView!.numberOfItems(inSection: 0)-1)*anglePerItem
            return angleAtExtreme
        } else {
            return -0.75
        }
    }
    
    var angle: CGFloat {
        let val = angleAtExtreme * collectionView!.contentOffset.x/(collectionViewContentSize.width - collectionView!.bounds.width)
        return val
    }
    
    var outerRadius: CGFloat = 400 {
        didSet {
            invalidateLayout()
        }
    }
    
    var middleRadius: CGFloat {
        get {
            return (self.outerRadius + self.innerRadius) / 2.0
        }
    }
    
    var innerRadius: CGFloat = 300 {
        didSet {
            invalidateLayout()
        }
    }
    
    var anglePerItem: CGFloat = 5
    
    var attributesList = [SelectorWheelRingLayoutAttributes]()
    
    override var collectionViewContentSize : CGSize {
        var num = collectionView!.numberOfItems(inSection: 0)
        if(num < 10) { num = 10 }
        return CGSize(width: CGFloat(num)*itemSize.width,
                      height: collectionView!.bounds.height)
    }
    
    override class var layoutAttributesClass : AnyClass {
        return SelectorWheelRingLayoutAttributes.self
    }
    
    override func prepare() {
        super.prepare()
        
        let numItems = collectionView!.numberOfItems(inSection: 0)
        
        if (numItems < 1 || anglePerItem <= 0) {
            return
        }
        
        let itemHeight = self.outerRadius - self.innerRadius
        
        let centerX = collectionView!.contentOffset.x + (collectionView!.frame.width/2.0)
        let anchorPointY = ((itemHeight/2.0) + self.middleRadius)/itemHeight
        //1
        let theta = atan2(collectionView!.frame.width/2.0, self.middleRadius + (itemHeight/2.0) - (collectionView!.frame.height/2.0)) //1
        //2
        var startIndex = 0
        var endIndex = numItems - 1
        //3
        if (angle < -theta) {
            startIndex = Int(floor((-theta - angle)/anglePerItem))
        }
        //4
        endIndex = min(endIndex, Int(ceil((theta - angle)/anglePerItem)))
        //5
        if (endIndex < startIndex) {
            endIndex = 0
            startIndex = 0
        }
        
        attributesList = (startIndex...endIndex).map { (i) -> SelectorWheelRingLayoutAttributes in
            let attributes = SelectorWheelRingLayoutAttributes(forCellWith: IndexPath(item: i, section: 0))
            attributes.size = CGSize(width: self.itemSize.width, height: itemHeight)
            attributes.center = CGPoint(x: centerX, y: self.collectionView!.frame.midY / 1.7 )
            attributes.angle = self.angle + (self.anglePerItem*CGFloat(i))
            attributes.anchorPoint = CGPoint(x: 0.5, y: anchorPointY)
            return attributes
        }
    }
    
    override func layoutAttributesForElements(in rect: CGRect) -> [UICollectionViewLayoutAttributes]? {
        return attributesList
    }
    
    override func layoutAttributesForItem(at indexPath: IndexPath)
        -> UICollectionViewLayoutAttributes? {
            if (attributesList.count > 0) {
                return attributesList[(indexPath as NSIndexPath).row % attributesList.count]
            }
            return nil
    }
    
    override func shouldInvalidateLayout(forBoundsChange newBounds: CGRect) -> Bool {
        return true
    }
    
    func offsetForRow(_ row:Int, _ centerOffset:Bool) -> CGFloat {
        var size = collectionViewContentSize.width - collectionView!.bounds.width
        if(size == 0) {
            size = 825
        }
        let factor = -angleAtExtreme/(size)
        if centerOffset {
            // Change the offset to be inbetween the two cells
            //print((CGFloat(row) * anglePerItem/factor) / 2)
            return (CGFloat(row) * anglePerItem/factor) / 2
        }
        return CGFloat(row) * anglePerItem/factor
    }
    
    func rowForOffset(_ offset:CGFloat) -> Int {
        let factor = -angleAtExtreme/(collectionViewContentSize.width - collectionView!.bounds.width)
        let apf = factor / anglePerItem
        return Int(0.5 + offset * apf)
    }
    
    //Uncomment the section below to activate snapping behavior
    override func targetContentOffset(forProposedContentOffset proposedContentOffset: CGPoint, withScrollingVelocity velocity: CGPoint) -> CGPoint {
        var finalContentOffset = proposedContentOffset
        let factor = -angleAtExtreme/(collectionViewContentSize.width - collectionView!.bounds.width)
        let proposedAngle = proposedContentOffset.x*factor
        let ratio = proposedAngle/anglePerItem
        var multiplier: CGFloat
        if (velocity.x > 0) {
            multiplier = ceil(ratio)
        } else if (velocity.x < 0) {
            multiplier = floor(ratio)
        } else {
            multiplier = round(ratio)
        }
        finalContentOffset.x = multiplier*anglePerItem/factor
        return finalContentOffset
    }
}

extension UIImage {
    
    func addImagePadding(x: CGFloat, y: CGFloat) -> UIImage {
        let width: CGFloat = self.size.width + x;
        let height: CGFloat = self.size.width + y;
        UIGraphicsBeginImageContextWithOptions(CGSize(width: width, height: height), false, 0);
        let context: CGContext = UIGraphicsGetCurrentContext()!;
        UIGraphicsPushContext(context);
        let origin: CGPoint = CGPoint(x: (width - self.size.width) / 2, y: (height - self.size.height) / 2);
        self.draw(at: origin)
        UIGraphicsPopContext();
        let imageWithPadding = UIGraphicsGetImageFromCurrentImageContext();
        UIGraphicsEndImageContext();
        
        return imageWithPadding!
    }
}
