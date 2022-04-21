// SPDX-License-Identifier: GPL-3.0

pragma solidity >=0.7.0 <0.9.0;


import "./Voter.sol";

contract Caster {
    address creator;
    address voterAddr;
    string[] public votes;
    
    constructor(address va) {
        creator = msg.sender;
        voterAddr = va;
    }

    function castVote(string memory cid) public {
        Voter v = Voter(voterAddr);
        v.removeVoter(msg.sender);

        votes.push(cid);
    }
}
