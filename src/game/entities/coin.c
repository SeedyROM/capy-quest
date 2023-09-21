#include "coin.h"

void CoinInit(Coin *coin, TextureAtlas *atlas)
{
    coin->collected = false;
    coin->atlas = atlas;
    coin->time = 0;
    coin->collectedTime = 0;
    coin->frameDuration = 10;
    coin->currentFrame = 0;
    coin->delete = false;

    SpriteFromAtlas(&coin->sprite, atlas, &STR("coin"));
}

void CoinCollect(Coin *coin)
{
    coin->collected = true;

    Vec2 pos = coin->sprite.pos;
    SpriteFromAtlas(&coin->sprite, coin->atlas, &STR("coin_collected"));
    coin->sprite.pos = pos;
    coin->currentFrame = 0;

    coin->frameDuration = 5;
    coin->time = 5;
}

void CoinUpdate(Coin *coin)
{
    if (coin->collected)
    {
        coin->collectedTime += 1;
        if (coin->collectedTime > 30)
        {
            coin->delete = true;
        }
    }
    if (coin->time % coin->frameDuration == 0)
    {
        coin->currentFrame = (coin->currentFrame + 1) % coin->sprite.frames.len;
    }
    coin->time += 1;
}
