/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
*/

#include "ScriptMgr.h"
#include "AuctionHouseBot.h"
#include "Log.h"
#include "Mail.h"
#include "Player.h"
#include "WorldSession.h"

class AHBot_WorldScript : public WorldScript
{
public:
    AHBot_WorldScript() : WorldScript("AHBot_WorldScript") { }

    void OnAfterConfigLoad(bool /*reload*/) override
    {
        auctionbot->InitializeConfiguration();
    }

    void OnStartup() override
    {
        LOG_INFO("server.loading", "Initialize AuctionHouseBot...");
        auctionbot->Initialize();
    }
};

class AHBot_AuctionHouseScript : public AuctionHouseScript
{
public:
    AHBot_AuctionHouseScript() : AuctionHouseScript("AHBot_AuctionHouseScript") { }

    void OnBeforeAuctionHouseMgrSendAuctionSuccessfulMail(AuctionHouseMgr* /*auctionHouseMgr*/, AuctionEntry* /*auction*/, Player* owner, uint32& /*owner_accId*/, uint32& /*profit*/, bool& sendNotification, bool& updateAchievementCriteria, bool& /*sendMail*/) override
    {
        if (owner)
        {
            bool isAHBot = false;
            for (AuctionHouseBotCharacter character : auctionbot->AHCharacters)
            {
                if (character.CharacterGUID == owner->GetGUID().GetCounter())
                {
                    isAHBot = true;
                    break;
                }
            }
            if (isAHBot == true)
            {
                sendNotification = false;
                updateAchievementCriteria = false;
            }
        }
    }

    void OnBeforeAuctionHouseMgrSendAuctionExpiredMail(AuctionHouseMgr* /*auctionHouseMgr*/, AuctionEntry* /*auction*/, Player* owner, uint32& /*owner_accId*/, bool& sendNotification, bool& /*sendMail*/) override
    {
        if (owner)
        {
            bool isAHBot = false;
            for (AuctionHouseBotCharacter character : auctionbot->AHCharacters)
            {
                if (character.CharacterGUID == owner->GetGUID().GetCounter())
                {
                    isAHBot = true;
                    break;
                }
            }
            if (isAHBot == true)
            {
                sendNotification = false;
            }
        }   
    }

    void OnBeforeAuctionHouseMgrSendAuctionOutbiddedMail(AuctionHouseMgr* /*auctionHouseMgr*/, AuctionEntry* auction, Player* oldBidder, uint32& /*oldBidder_accId*/, Player* newBidder, uint32& newPrice, bool& /*sendNotification*/, bool& /*sendMail*/) override
    {
        if (oldBidder && !newBidder)
            oldBidder->GetSession()->SendAuctionBidderNotification((uint32)auction->GetHouseId(), auction->Id, ObjectGuid::Create<HighGuid::Player>(auctionbot->CurrentBotCharGUID), newPrice, auction->GetAuctionOutBid(), auction->item_template);
    }

    void OnBeforeAuctionHouseMgrUpdate() override
    {
        auctionbot->Update();
    }
};

class AHBot_MailScript : public MailScript
{
public:
    AHBot_MailScript() : MailScript("AHBot_MailScript") { }

    void OnBeforeMailDraftSendMailTo(MailDraft* /*mailDraft*/, MailReceiver const& receiver, MailSender const& sender, MailCheckMask& /*checked*/, uint32& /*deliver_delay*/, uint32& /*custom_expiration*/, bool& deleteMailItemsFromDB, bool& sendMail) override
    {
        bool isAHBot = false;
        for (AuctionHouseBotCharacter character : auctionbot->AHCharacters)
        {
            if (character.CharacterGUID == receiver.GetPlayerGUIDLow())
            {
                isAHBot = true;
                break;
            }
        }
        if (isAHBot == true)
        {
            if (sender.GetMailMessageType() == MAIL_AUCTION)        // auction mail with items
                deleteMailItemsFromDB = true;
            sendMail = false;
        }
    }
};

void AddAHBotScripts()
{
    new AHBot_WorldScript();
    new AHBot_AuctionHouseScript();
    new AHBot_MailScript();
}
