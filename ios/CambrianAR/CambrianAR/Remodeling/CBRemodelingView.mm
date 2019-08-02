//
//  CBRemodelingView.m
//  CambrianAR
//
//  Created by Joel Teply on 3/3/17.
//  Copyright © 2017 Joel Teply. All rights reserved.
//

#import "CBRemodelingView.h"
#import "CBARInternal.h"
#import <imaging/ImageProcessing.h>
#import "ImageTranslation.h"

@implementation CBRemodelingView

@dynamic scene;
@dynamic coreView;

- (void) generateScene {
    self.scene = [CBRemodelingScene new];
}

- (std::shared_ptr<CBAR_View>) generateCoreView:(std::shared_ptr<CBAR_CallbackI>)callback; {
    auto coreView = std::shared_ptr<CBAR_View>(new CBAR_View(callback));
    coreView->setToolMode(ToolModeFill);
    return coreView;
}

- (void)setDelegate:(id<CBRemodelingViewDelegate>)delegate {
    super.rootDelegate = delegate;
}

- (void) historyChanged:(std::shared_ptr<cbpipe::UndoState>)undo forward:(BOOL)forward {
    
    if (![self.delegate respondsToSelector:@selector(historyChanged:change:forward:)]) {
        return;
    }
    
    CBUndoChange change = CBUndoChangeMask;
    
    switch (undo->change) {
        case cbpipe::undo_change_mask:
            change = CBUndoChangeMask;
            break;
        case cbpipe::undo_change_paint_color:
            change = CBUndoChangePaintColor;
            break;
        case cbpipe::undo_change_paint_sheen:
            change = CBUndoChangePaintSheen;
            break;
        default:
            break;
    }
    
    NSString *assetID = [NSString stringWithUTF8String:undo->assetID.c_str()];
    CBAugmentedAsset *asset = [self.scene.assets objectForKey:assetID];
    
    __weak typeof(self) weakSelf = self;
    dispatch_async(dispatch_get_main_queue(), ^{
        __strong typeof(self) strongSelf = weakSelf; if (!strongSelf || !strongSelf.delegate) return;
        [strongSelf.delegate historyChanged:asset change:change forward:forward];
    });
}

@end
