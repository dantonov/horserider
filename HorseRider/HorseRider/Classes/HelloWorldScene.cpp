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

#include "HelloWorldScene.h"
#include "SimpleAudioEngine.h"
#include <sstream>
#include <map>
#include <set>

USING_NS_CC;

static const std::string sEmptyCellImageName("res/empty.png");
static const std::string sBlockedCellImageName("res/tower.png");
static const std::string sHorseCellImageName("res/knight.png");
static const std::string sHorseRedImageName("res/knight_red.png");
static const std::string sTargetCellImageName("res/carrot.png");
static const std::string sRunButtonImageName("CloseSelected.png");
static const ItemIndex sSideLen = 8;


Scene* HelloWorld::createScene()
{
    return HelloWorld::create();
}

// Print useful error message instead of segfaulting when files are not there.
static void problemLoading(const char* filename)
{
    printf("Error while loading: %s\n", filename);
    printf("Depending on how you compiled you might have to add 'Resources/' in front of filenames in HelloWorldScene.cpp\n");
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    if ( !Scene::init() )
    {
        return false;
    }

    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

//    auto closeItem = MenuItemImage::create(
//                                           "CloseNormal.png",
//                                           "CloseSelected.png",
//                                           CC_CALLBACK_1(HelloWorld::menuCloseCallback, this));

//    if (closeItem == nullptr ||
//        closeItem->getContentSize().width <= 0 ||
//        closeItem->getContentSize().height <= 0)
//    {
//        problemLoading("'CloseNormal.png' and 'CloseSelected.png'");
//    }
//    else
//    {
//        float x = origin.x + visibleSize.width - closeItem->getContentSize().width/2;
//        float y = origin.y + closeItem->getContentSize().height/2;
//        closeItem->setPosition(Vec2(x,y));
//    }

    // create menu, it's an autorelease object
//    auto menu = Menu::create(closeItem, NULL);
//    menu->setPosition(Vec2::ZERO);
//    this->addChild(menu, 1);
//
    mErrorText = Label::createWithTTF("Path not found!", "fonts/Marker Felt.ttf", 24);
    if (mErrorText == nullptr)
    {
        problemLoading("'fonts/Marker Felt.ttf'");
    }
    else
    {
        // position the label on the center of the screen
        mErrorText->setPosition(Vec2(origin.x + visibleSize.width/2,
                                origin.y + visibleSize.height - mErrorText->getContentSize().height));

        // add the label as a child to this layer
        addChild(mErrorText, 1);
        mErrorText->setVisible(false);
    }
//
//    auto sprite = Sprite::create("HelloWorld.png");
//    if (sprite == nullptr)
//    {
//        problemLoading("'HelloWorld.png'");
//    }
//    else
//    {
//        // position the sprite on the center of the screen
//        sprite->setPosition(Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));
//
//        // add the sprite as a child to this layer
//        this->addChild(sprite, 0);
//    }
    createBoard(origin, visibleSize);
    createControlButtons(origin, visibleSize);
    return true;
}

void HelloWorld::menuCloseCallback(Ref* pSender)
{
    Director::getInstance()->end();
}

void HelloWorld::onTypeSelected(cocos2d::Ref* pSender)
{
    cocos2d::Node* node = dynamic_cast<cocos2d::Node*>(pSender);
    if (!node)
        return;
    
    printf("Selected type = %d\n", node->getTag());
    mCurrentlySelectedItemTag = node->getTag();
}

void HelloWorld::onItemClicked(cocos2d::Ref* pSender)
{
    cocos2d::MenuItemToggle* Item = dynamic_cast<cocos2d::MenuItemToggle*>(pSender);
    if (!Item)
        return;

    printf("Item clicked %d\n", Item->getTag());
    Item->setSelectedIndex(mCurrentlySelectedItemTag);
    if (mCurrentlySelectedItemTag == ItemType::Knight) // knight
    {
        if (mCurrentHorseItem)
            mCurrentHorseItem->setSelectedIndex(0);
        mCurrentHorseItem = Item;
    }
    else if (mCurrentlySelectedItemTag == ItemType::Target) // Target
    {
        if (mCurrentTargetItem)
            mCurrentTargetItem->setSelectedIndex(0);
        mCurrentTargetItem = Item;
    }
}

void HelloWorld::onRun(cocos2d::Ref* pSender)
{
    std::vector<ItemType> items;
    ItemIndex horseId = -1;
    ItemIndex targetId = -1;
    prepareItems(&items, &horseId, &targetId);
    if (items.size() == 0 || horseId == -1 || targetId == -1)
        return;

    printf("Horse %d target %d\n", horseId, targetId);
    std::vector<ItemIndex> path = findPath(items, horseId, targetId);
    if (path.size())
        path.push_back(targetId);
    printf("Path: \n");
    for (ItemIndex index : path)
        printf("%d ", index);

    animatePath(path);
}

void HelloWorld::createBoard(const Vec2& origin, const Size& size)
{
    static const std::string sEmptyCellImageName("res/empty.png");
    static const std::string sBlockedCellImageName("res/tower.png");
    static const std::string sHorseCellImageName("res/knight.png");
    static const std::string sTargetCellImageName("res/carrot.png");

    float imageW = -1.0f;
    float imageH = -1.0f;
    float boardBeginX = -1.0f;
    float boardBeginY = -1.0f;

    for (unsigned i = 0; i < sSideLen * sSideLen; ++i)
    {
        MenuItemSprite* spriteEmpty = MenuItemImage::create(sEmptyCellImageName, sEmptyCellImageName);
        MenuItemSprite* spriteBlocked = MenuItemImage::create(sBlockedCellImageName, sBlockedCellImageName);
        MenuItemSprite* spriteHorse = MenuItemImage::create(sHorseCellImageName, sHorseCellImageName);
        MenuItemSprite* spriteTarget = MenuItemImage::create(sTargetCellImageName, sTargetCellImageName);
        if (!spriteEmpty || !spriteBlocked || !spriteHorse || !spriteTarget)
        {
            printf("Error while loading one of this: %s %s %s %s\n", sEmptyCellImageName.c_str(), sBlockedCellImageName.c_str(), sHorseCellImageName.c_str(), sTargetCellImageName.c_str());
            break;
        }

        Size spriteSize = spriteEmpty->getContentSize();
        if (imageW < 0.0f)
            imageW = spriteSize.width;
        if (imageH < 0.0f)
            imageH = spriteSize.height;

        std::stringstream ss;
        ss << i;
        Label* label = Label::createWithTTF(ss.str().c_str(), "fonts/Marker Felt.ttf", 24);
        if (label)
            spriteEmpty->addChild(label, 1);

        if (boardBeginX < 0.0f)
            boardBeginX = (size.width - sSideLen * imageW) * 0.5f;
        if (boardBeginY < 0.0f)
            boardBeginY = (size.height - sSideLen * imageH) * 0.5f;

        cocos2d::MenuItemToggle* toggleImage = cocos2d::MenuItemToggle::createWithCallback(CC_CALLBACK_1(HelloWorld::onItemClicked, this), spriteEmpty, spriteBlocked, spriteHorse, spriteTarget, nullptr);

        toggleImage->setPosition(Vec2(boardBeginX + imageW * (i % sSideLen), boardBeginY + imageH * (i / sSideLen)));
        toggleImage->setSelectedIndex(0);

        toggleImage->setTag(i);
        mItems.pushBack(toggleImage);
    }

    cocos2d::Menu* menu = cocos2d::Menu::createWithArray(mItems);
    menu->setPosition(Vec2::ZERO);
    addChild(menu, 1);
}

void HelloWorld::createControlButtons(const cocos2d::Vec2& origin, const cocos2d::Size& size)
{
    float currentY = size.height * 0.2f;
    MenuItemSprite* spriteEmpty = MenuItemImage::create(sEmptyCellImageName, sEmptyCellImageName, CC_CALLBACK_1(HelloWorld::onTypeSelected, this));
    if (!spriteEmpty)
        return;
    spriteEmpty->setTag(ItemType::Empty);
    spriteEmpty->setPosition(cocos2d::Vec2(size.width - spriteEmpty->getContentSize().width, currentY));
    currentY += spriteEmpty->getContentSize().height;

    MenuItemSprite* spriteBlocked = MenuItemImage::create(sBlockedCellImageName, sBlockedCellImageName, CC_CALLBACK_1(HelloWorld::onTypeSelected, this));
    if (!spriteBlocked)
        return;
    spriteBlocked->setTag(ItemType::Tower);
    spriteBlocked->setPosition(cocos2d::Vec2(size.width - spriteBlocked->getContentSize().width, currentY));
    currentY += spriteBlocked->getContentSize().height;

    MenuItemSprite* spriteHorse = MenuItemImage::create(sHorseCellImageName, sHorseCellImageName, CC_CALLBACK_1(HelloWorld::onTypeSelected, this));
    if (!spriteHorse)
        return;
    spriteHorse->setTag(ItemType::Knight);
    spriteHorse->setPosition(cocos2d::Vec2(size.width - spriteHorse->getContentSize().width, currentY));
    currentY += spriteHorse->getContentSize().height;

    MenuItemSprite* spriteTarget = MenuItemImage::create(sTargetCellImageName, sTargetCellImageName, CC_CALLBACK_1(HelloWorld::onTypeSelected, this));
    if (!spriteTarget)
        return;
    spriteTarget->setTag(ItemType::Target);
    spriteTarget->setPosition(cocos2d::Vec2(size.width - spriteTarget->getContentSize().width, currentY));
    currentY += spriteTarget->getContentSize().height;

    MenuItemSprite* spriteRun = MenuItemImage::create(sRunButtonImageName, sRunButtonImageName, CC_CALLBACK_1(HelloWorld::onRun, this));
    if (!spriteRun)
        return;
    spriteRun->setPosition(cocos2d::Vec2(size.width - spriteRun->getContentSize().width, currentY));
    currentY += spriteRun->getContentSize().height;

    cocos2d::Menu* menu = cocos2d::Menu::create(spriteEmpty, spriteBlocked, spriteHorse, spriteTarget, spriteRun, nullptr);
    if (!menu)
        return;
    menu->setPosition(Vec2::ZERO);
    addChild(menu, 2);
}

void HelloWorld::prepareItems(std::vector<ItemType>* outItems, ItemIndex* outHorse, ItemIndex* outTarget)
{
    for (const cocos2d::MenuItem* item : mItems)
    {
        const cocos2d::MenuItemToggle* toggleItem = dynamic_cast<const cocos2d::MenuItemToggle*>(item);
        ItemType type = (ItemType)toggleItem->getSelectedIndex();
        outItems->push_back(type);
        if (type == ItemType::Knight)
            *outHorse = (ItemIndex)outItems->size() - 1;
        else if (type == ItemType::Target)
            *outTarget = (ItemIndex)outItems->size() - 1;
    }
}

std::vector<ItemIndex> HelloWorld::findPath(std::vector<ItemType>& items, ItemIndex horseId, ItemIndex targetId)
{
    std::vector<Item> newAvailable = getAvailableItemsFor(Item(horseId, {}), items);

    return processItemsFor(newAvailable, items);
}

std::vector<ItemIndex> HelloWorld::processItemsFor(const std::vector<Item>& processingItems, std::vector<ItemType>& items)
{
    if (processingItems.size() == 0)
        return {};

    std::vector<Item> nextStepProcessing;
    for (const Item& item : processingItems)
    {
//        if (items[item.itemIndex] == ItemType::Target)
//            return item.path;

        std::vector<Item> newAvailable = getAvailableItemsFor(item, items);
        if (newAvailable.size() != 0)
            nextStepProcessing.insert(nextStepProcessing.end(), newAvailable.begin(), newAvailable.end());
    }

    for (Item& item : nextStepProcessing)
    {
        if (items[item.itemIndex] == ItemType::Target)
            return item.path;

        items[item.itemIndex] = ItemType::Processed;
    }

    return processItemsFor(nextStepProcessing, items);
}

std::vector<HelloWorld::Item> HelloWorld::getAvailableItemsFor(const Item& inItem, const std::vector<ItemType>& items)
{
    ItemIndex x = getX(inItem.itemIndex);
    ItemIndex y = getY(inItem.itemIndex);

    std::vector<std::pair<ItemIndex, ItemIndex>> points;
    if (x > 1)
    {
        if (y > 0)
            points.push_back(std::make_pair(x - 2, y - 1));
        if (y < sSideLen - 1)
            points.push_back(std::make_pair(x - 2, y + 1));
    }
    if (x < sSideLen - 2)
    {
        if (y > 0)
            points.push_back(std::make_pair(x + 2, y - 1));
        if (y < sSideLen - 1)
            points.push_back(std::make_pair(x + 2, y + 1));
    }
    if (y > 1)
    {
        if (x > 0)
            points.push_back(std::make_pair(x - 1, y - 2));
        if (x < sSideLen - 1)
            points.push_back(std::make_pair(x + 1, y - 2));
    }
    if (y < sSideLen - 2)
    {
        if (x > 0)
            points.push_back(std::make_pair(x - 1, y + 2));
        if (x < sSideLen - 1)
            points.push_back(std::make_pair(x + 1, y + 2));
    }

    std::vector<Item> outItems;
    for (auto& pair : points)
    {
        ItemIndex itemId = pair.first + pair.second * sSideLen;
        if (items[itemId] == ItemType::Tower || items[itemId] == ItemType::Processed || items[itemId] == ItemType::Knight)
            continue;

        std::vector<ItemIndex> path = inItem.path;
        path.push_back(inItem.itemIndex);
        outItems.push_back(std::move(Item(itemId, path)));
    }

    printf("OUT for %d:%d(%d) :\n", x, y, inItem.itemIndex);
    for (const Item& item : outItems)
    {
        printf("%d ", item.itemIndex);
    }
    printf("\n");

    return outItems;
}

ItemIndex HelloWorld::getX(ItemIndex index)
{
    return index % sSideLen;
}

ItemIndex HelloWorld::getY(ItemIndex index)
{
    return index / sSideLen;
}

void HelloWorld::animatePath(const std::vector<ItemIndex>& path)
{
    if (mErrorText)
        mErrorText->setVisible(false);

    if (mActionControlledSprite)
    {
        mActionControlledSprite->stopAllActions();
        mActionControlledSprite->removeFromParentAndCleanup(true);
        mActionControlledSprite = nullptr;
    }

    if (!path.size())
    {
        if (mErrorText)
            mErrorText->setVisible(true);
        return;
    }

    mActionControlledSprite = Sprite::create(sHorseRedImageName);
    addChild(mActionControlledSprite, 21);
    mActionControlledSprite->setPosition(mItems.at(path[0])->getPosition());

    Vector<FiniteTimeAction*> arrayOfActions;
    for (ItemIndex index : path)
    {
        arrayOfActions.pushBack(MoveTo::create(1, mItems.at(index)->getPosition()));
        arrayOfActions.pushBack(DelayTime::create(0.5f));
    }
    arrayOfActions.pushBack(Hide::create());

    Sequence* seq = Sequence::create(arrayOfActions);
    mActionControlledSprite->runAction(seq);
}
