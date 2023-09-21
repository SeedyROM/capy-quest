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
    if (coin->collected)
        return;

    coin->collected = true;

    SpriteChange(&coin->sprite, &STR("coin_collected"));
    coin->currentFrame = 0;

    coin->frameDuration = 1;
    coin->time = 0;
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
