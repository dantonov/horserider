/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 
 http://www.cocos2d-x.org
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"
#include <vector>

enum ItemType : short
{
    Empty = 0,
    Tower = 1,
    Knight = 2,
    Target = 3,
    Processed = 11
};

typedef int ItemIndex;

class HelloWorld : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();

    virtual bool init();
    
    void menuCloseCallback(cocos2d::Ref* pSender);

    void onTypeSelected(cocos2d::Ref* pSender);
    void onItemClicked(cocos2d::Ref* pSender);
    void onRun(cocos2d::Ref* pSender);

    CREATE_FUNC(HelloWorld);

private:
    struct Item
    {
        inline Item(ItemIndex i, const std::vector<ItemIndex>& p) : itemIndex(i), path(p) {}
        inline bool operator<(const Item& item) { return itemIndex < item.itemIndex; }
        inline bool operator==(const Item& item) { return itemIndex == item.itemIndex; }

        ItemIndex itemIndex = -1;
        std::vector<ItemIndex> path;
    };

    cocos2d::Vector<cocos2d::MenuItem*> mItems;
    cocos2d::MenuItemToggle* mCurrentHorseItem = nullptr;
    cocos2d::MenuItemToggle* mCurrentTargetItem = nullptr;
    ItemIndex mCurrentlySelectedItemTag = 0;
    cocos2d::Sprite* mActionControlledSprite = nullptr;
    cocos2d::Label* mErrorText = nullptr;

    void createBoard(const cocos2d::Vec2& origin, const cocos2d::Size& size);
    void createControlButtons(const cocos2d::Vec2& origin, const cocos2d::Size& size);
    void prepareItems(std::vector<ItemType>* outItems, ItemIndex* outHorse, ItemIndex* outTarget);
    std::vector<ItemIndex> findPath(std::vector<ItemType>& items, ItemIndex horseId, ItemIndex targetId);
    std::vector<ItemIndex> processItemsFor(const std::vector<Item>& processingItems, std::vector<ItemType>& items);
    std::vector<Item> getAvailableItemsFor(const Item& index, const std::vector<ItemType>& items);
    ItemIndex getX(ItemIndex index);
    ItemIndex getY(ItemIndex index);

    void animatePath(const std::vector<ItemIndex>& path);
};

#endif // __HELLOWORLD_SCENE_H__
