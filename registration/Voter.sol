// SPDX-License-Identifier: GPL-3.0

pragma solidity >=0.7.0 <0.9.0;


contract Voter {
   
    address public creator;
    address public voteSubmitter;
    mapping(address => bool) public key_generators;
    bytes[] keys;

    
    constructor() {
        creator = msg.sender;
        voteSubmitter = msg.sender;
    }

    function registerGenerator(address addr) public {
        require(
            creator == msg.sender,
            "Unauthorized"
        );
        
        require(
            !key_generators[addr],
            "Generator already exists"
        );

        key_generators[addr] = true;
    }

    function registerVoter(bytes memory key) public {
        require(
            key_generators[msg.sender],
            "Key Generator not registered"
        );

        keys.push(key);
    }

    function registerVoteSubmitter(address vs) public {
        require(
            creator == msg.sender,
            "Unauthorized"
        );

        voteSubmitter = vs;
    }

    function removeVoter(bytes memory key) public {
        require(
            msg.sender == creator || msg.sender == voteSubmitter,
            "Unauthorized"
        );

        bool found = false;
        uint idx = 0;
        for (uint i = 0; i < keys.length; i++){
            if (keccak256(keys[i]) == keccak256(key)){
                found = true;
                idx = i;
            }
        }

        require(
            found,
            "Key does not exist"
        );

        keys[idx] = keys[keys.length - 1];
        delete keys[keys.length - 1];
    }

    function isVoter(bytes memory key) view public returns(bool) {
        bool found = false;

        for (uint i = 0; i < keys.length; i++){
            if (keccak256(keys[i]) == keccak256(key)){
                found = true;
            }
        }

        return found;
    }
    
}
