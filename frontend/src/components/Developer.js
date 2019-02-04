import React, { Component } from 'react';
import HexViewer from './HexViewer';
import SourceEditor from './SourceEditor';

class Developer extends Component {
    render() {
        return (<div>
            <SourceEditor />
            <button>Compile</button>
            <HexViewer />
        </div>);
    }
}

export default Developer;