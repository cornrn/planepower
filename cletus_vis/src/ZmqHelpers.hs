{-# OPTIONS_GHC -Wall #-}
{-# Language ScopedTypeVariables #-}
--{-# Language DeriveFunctor #-}
--{-# Language DeriveGeneric #-}
{-# Language FlexibleInstances #-}
{-# Language UndecidableInstances #-}

module ZmqHelpers
       ( Packed
       , ZMQ.Context
       , ZMQ.withContext
       , withPublisher
       , withSubscriber
       , encodeProto
       , encodeSerial
       , decodeProto
       , decodeSerial
       ) where

--import Linear
--import Data.Vector ( Vector )
--import GHC.Generics ( Generic )
--
--import Dyno.Vectorize
--import Dyno.View
--import Dyno.Ipopt
----import Dyno.Snopt
--import Dyno.Nlp
--import Dyno.NlpSolver

import qualified Data.ByteString as BS
import qualified Data.ByteString.Lazy as BSL
import qualified Data.ByteString.Char8 as BS8
import qualified Data.Serialize as Ser
import qualified System.ZMQ as ZMQ
import qualified Text.ProtocolBuffers as PB

--callback :: (Serialize a) => ZMQ.Socket ZMQ.Pub -> String -> a -> IO Bool
--callback publisher chanName stuff = do
--  let bs = encode stuff
--  ZMQ.send publisher [ZMQ.SendMore] (pack chanName)
--  ZMQ.send publisher [] bs
--  return True
--
--withCallback :: (Serialize a) => String -> String -> (((DynCollTraj a, CollTrajMeta) -> IO Bool) -> IO b) -> IO b
--withCallback url channelName userFun =
--  ZMQ.withContext $ \context ->
--    ZMQ.withSocket context ZMQ.Pub $ \publisher ->
--      ZMQ.bind publisher url >> userFun (callback publisher channelName)

newtype Packed = Packed { unPacked :: BS.ByteString }

fromStrict :: BS.ByteString -> BSL.ByteString
fromStrict x = BSL.fromChunks [x]

toStrict :: BSL.ByteString -> BS.ByteString
toStrict = BS.concat . BSL.toChunks

sendMulti :: ZMQ.Socket ZMQ.Pub -> String -> BS.ByteString -> IO ()
sendMulti publisher chanName bs = do
  ZMQ.send publisher (BS8.pack chanName) [ZMQ.SndMore]
  ZMQ.send publisher bs []

receiveMulti :: ZMQ.Socket ZMQ.Sub -> IO [BS.ByteString]
receiveMulti subscriber = do
  r0 <- ZMQ.receive subscriber []
  return [r0]
{-  mre <- ZMQ.moreToReceive subscriber
  if mre
  then do
    r1 <- ZMQ.receive subscriber []
    return [r0,r1]
  else do
    error $ "not moar to receive lol: " ++ show r0
-}

encodeProto :: (PB.ReflectDescriptor a, PB.Wire a) => a -> Packed
encodeProto = Packed . toStrict . PB.messagePut

encodeSerial :: Ser.Serialize a => a -> Packed
encodeSerial = Packed . Ser.encode

decodeProto :: (PB.ReflectDescriptor a, PB.Wire a) => Packed -> Either String a
decodeProto = fmap fst . PB.messageGet . fromStrict . unPacked

decodeSerial :: Ser.Serialize a => Packed -> Either String a
decodeSerial = Ser.decode . unPacked

withPublisher
  :: ZMQ.Context -> String -> ((String -> Packed -> IO ()) -> IO a) -> IO a
withPublisher context url f =
  ZMQ.withSocket context ZMQ.Pub $ \publisher -> do
    ZMQ.bind publisher url
    let send :: String -> Packed -> IO ()
        send channel msg =
          sendMulti publisher channel (unPacked msg)
    f send


withSubscriber
  :: ZMQ.Context -> String -> String -> (IO Packed -> IO a) -> IO a
withSubscriber context url channel f =
  ZMQ.withSocket context ZMQ.Sub $ \subscriber -> do
    ZMQ.connect subscriber url
    ZMQ.subscribe subscriber channel
    putStrLn "wooo"
    let receive = do
          msg <- receiveMulti subscriber :: IO [BS.ByteString]
         -- channel':msg <- receiveMulti subscriber :: IO [BS.ByteString]
          --unless ((BS8.unpack channel') == channel) $ error $ "bad channel: \"" ++ BS8.unpack channel' ++ "\""
          return (Packed (BS.concat msg))

    f receive
