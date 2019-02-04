import React, { Component } from 'react';
import HexViewer from './HexViewer';
import SourceEditor from './SourceEditor';

class Developer extends Component {

    outputBinary = Uint8Array.of(1, 2, 3);

    state = {}

    compile() {
        
    }

    render() {
        return (<div>
            <SourceEditor />
            <button>Compile</button>
            <HexViewer binary={this.outputBinary} />
        </div>);
    }
}

export default Developer;